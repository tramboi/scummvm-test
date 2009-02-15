/***************************************************************************
 Copyright (C) 2005 Christoph Reichenbach <reichenb@colorado.edu>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.

***************************************************************************/

#include "gc.h"

#define WORKLIST_CHUNK_SIZE 32

/*#define DEBUG_GC*/
/*#define DEBUG_GC_VERBOSE*/

typedef struct _worklist {
	int used;
	reg_t entries[WORKLIST_CHUNK_SIZE];
	struct _worklist *next;
} worklist_t;

static worklist_t *
fresh_worklist(worklist_t *old)
{
	worklist_t *retval = (worklist_t*)sci_malloc(sizeof(worklist_t));
	retval->used = 0;
	retval->next = old;
	return retval;
}

static worklist_t *
new_worklist()
{
	return fresh_worklist(NULL);
}

static void
worklist_push(worklist_t **wlp, reg_t_hash_map_ptr hashmap, reg_t reg)
{
	worklist_t *wl = *wlp;
	char added;

	if (!reg.segment) /* No numbers */
		return;

#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] Adding "PREG"\n", PRINT_REG(reg));
#endif

	reg_t_hash_map_check_value(hashmap, reg, 1, &added);

	if (!added)
		return; /* already dealt with it */

	if (!wl || wl->used == WORKLIST_CHUNK_SIZE)
		*wlp = wl = fresh_worklist(wl);

	wl->entries[wl->used++] = reg;
}

static int
worklist_has_next(worklist_t *wl)
{
	return (wl && wl->used);
}

static reg_t
worklist_pop(worklist_t **wlp)
{
	worklist_t *wl = *wlp;
	reg_t retval;

	if (!wl || !wl->used) {
		fprintf(stderr, "Attempt to pop from empty worklist");
		exit(1);
	}

	retval = wl->entries[--wl->used];

	if (!wl->used) {
		*wlp = wl->next;
		sci_free(wl);
	}

	return retval;
}

static void
free_worklist(worklist_t *wl)
{
	if (wl) {
		if (wl->next)
			free_worklist(wl->next);
		sci_free(wl);
	}
}

typedef struct {
	seg_interface_t **interfaces;
	int interfaces_nr;
	reg_t_hash_map_ptr normal_map;
} normaliser_t;

void
store_normalised(void *pre_normaliser, reg_t reg, int _)
{
	seg_interface_t *interfce;
	normaliser_t *normaliser = (normaliser_t *) pre_normaliser;
	interfce = (reg.segment < normaliser->interfaces_nr)
		? normaliser->interfaces[reg.segment]
		: NULL;

	if (interfce) {
		reg = interfce->find_canonic_address(interfce, reg);
		reg_t_hash_map_check_value(normaliser->normal_map, reg, 1, NULL);
	}
}

static reg_t_hash_map_ptr
normalise_hashmap_ptrs(reg_t_hash_map_ptr nonnormal_map, seg_interface_t **interfaces, int interfaces_nr)
{
	normaliser_t normaliser;

	normaliser.normal_map = new_reg_t_hash_map();
	normaliser.interfaces_nr = interfaces_nr;
	normaliser.interfaces = interfaces;
	apply_to_reg_t_hash_map(nonnormal_map, &normaliser, &store_normalised);

	return normaliser.normal_map;
}


typedef struct {
	reg_t_hash_map_ptr nonnormal_map;
	worklist_t **worklist_ref;
} worklist_manager_t;

void
add_outgoing_refs(void *pre_wm, reg_t addr)
{
	worklist_manager_t *wm = (worklist_manager_t *) pre_wm;
	worklist_push(wm->worklist_ref, wm->nonnormal_map, addr);
}

reg_t_hash_map_ptr
find_all_used_references(state_t *s)
{
	seg_manager_t *sm = &(s->seg_manager);
	seg_interface_t **interfaces = (seg_interface_t**)sci_calloc(sizeof(seg_interface_t *), sm->heap_size);
	reg_t_hash_map_ptr nonnormal_map = new_reg_t_hash_map();
	reg_t_hash_map_ptr normal_map = NULL;
	worklist_t *worklist = new_worklist();
	worklist_manager_t worklist_manager;
	int i;

	worklist_manager.worklist_ref = &worklist;
	worklist_manager.nonnormal_map = nonnormal_map;

	for (i = 1; i < sm->heap_size; i++)
		if (sm->heap[i] == NULL)
			interfaces[i] = NULL;
		else
			interfaces[i] = get_seg_interface(sm, i);

	/* Initialise */
	/* Init: Registers */
	worklist_push(&worklist, nonnormal_map, s->r_acc);
	worklist_push(&worklist, nonnormal_map, s->r_prev);
	/* Init: Value Stack */
	/* We do this one by hand since the stack doesn't know the current execution stack */
	{
		exec_stack_t *xs = s->execution_stack + s->execution_stack_pos;
		reg_t *pos;

		for (pos = s->stack_base; pos < xs->sp; pos++)
			worklist_push(&worklist, nonnormal_map, *pos);
	}
#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] -- Finished adding value stack");
#endif


	/* Init: Execution Stack */
	for (i = 0; i <= s->execution_stack_pos; i++) {
		exec_stack_t *es = s->execution_stack + i;

		if (es->type != EXEC_STACK_TYPE_KERNEL) {
			worklist_push(&worklist, nonnormal_map, es->objp);
			worklist_push(&worklist, nonnormal_map, es->sendp);
			if (es->type == EXEC_STACK_TYPE_VARSELECTOR)
				worklist_push(&worklist, nonnormal_map, *(es->addr.varp));
		}
	}
#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] -- Finished adding execution stack");
#endif

	/* Init: Explicitly loaded scripts */
	for (i = 1; i < sm->heap_size; i++)
		if (interfaces[i]
		    && interfaces[i]->type_id == MEM_OBJ_SCRIPT) {
			script_t *script = &(interfaces[i]->mobj->data.script);

			if (script->lockers) { /* Explicitly loaded? */
				int obj_nr;

				/* Locals, if present */
				worklist_push(&worklist, nonnormal_map, make_reg(script->locals_segment, 0));

				/* All objects (may be classes, may be indirectly reachable) */
				for (obj_nr = 0; obj_nr < script->objects_nr; obj_nr++) {
					object_t *obj = script->objects + obj_nr;
					worklist_push(&worklist,
						      nonnormal_map,
						      obj->pos);
				}
			}
		}
#ifdef DEBUG_GC_VERBOSE
	sciprintf("[GC] -- Finished explicitly loaded scripts, done with root set");
#endif


	/* Run Worklist Algorithm */
	while (worklist_has_next(worklist)) {
		reg_t reg = worklist_pop(&worklist);
		if (reg.segment != s->stack_segment) { /* No need to repeat this one */
#ifdef DEBUG_GC_VERBOSE
			sciprintf("[GC] Checking "PREG"\n", PRINT_REG(reg)); 
#endif
			if (reg.segment < sm->heap_size
			    && interfaces[reg.segment])
				interfaces[reg.segment]->list_all_outgoing_references(interfaces[reg.segment],
										      s,
										      reg,
										      &worklist_manager,
										      add_outgoing_refs);
		}
	}

	/* Normalise */
	normal_map = normalise_hashmap_ptrs(nonnormal_map, interfaces, sm->heap_size);

	/* Cleanup */
	for (i = 1; i < sm->heap_size; i++)
		if (interfaces[i])
			interfaces[i]->deallocate_self(interfaces[i]);
	sci_free(interfaces);
	free_reg_t_hash_map(nonnormal_map);
	return normal_map;
}


typedef struct {
	seg_interface_t *interfce;
#ifdef DEBUG_GC
	char *segnames[MEM_OBJ_MAX + 1];
	int segcount[MEM_OBJ_MAX + 1];
#endif
	reg_t_hash_map_ptr use_map;
} deallocator_t;

void
free_unless_used (void *pre_use_map, reg_t addr)
{
	deallocator_t *deallocator = (deallocator_t *) pre_use_map;
	reg_t_hash_map_ptr use_map = deallocator->use_map;

	if (0 > reg_t_hash_map_check_value(use_map, addr, 0, NULL)) {
		/* Not found -> we can free it */
		deallocator->interfce->free_at_address(deallocator->interfce, addr);
#ifdef DEBUG_GC
		sciprintf("[GC] Deallocating "PREG"\n", PRINT_REG(addr));
		deallocator->segcount[deallocator->interfce->type_id]++;
#endif
	}

}

void
run_gc(state_t *s)
{
	int seg_nr;
	deallocator_t deallocator;
	seg_manager_t *sm = &(s->seg_manager);

#ifdef DEBUG_GC
	c_segtable(s);
	sciprintf("[GC] Running...\n");
	memset(&(deallocator.segcount), 0, sizeof(int) * (MEM_OBJ_MAX + 1));
#endif

	deallocator.use_map = find_all_used_references(s);

	for (seg_nr = 1; seg_nr < sm->heap_size; seg_nr++)
		if (sm->heap[seg_nr] != NULL) {
			deallocator.interfce = get_seg_interface(sm, seg_nr);
#ifdef DEBUG_GC
			deallocator.segnames[deallocator.interfce->type_id] = deallocator.interfce->type;
#endif

			deallocator.interfce->list_all_deallocatable(deallocator.interfce,
								     &deallocator,
								     free_unless_used);

			deallocator.interfce->deallocate_self(deallocator.interfce);
		}

	free_reg_t_hash_map(deallocator.use_map);

#ifdef DEBUG_GC
	{
		int i;
		sciprintf("[GC] Summary:\n");
		for (i = 0; i <= MEM_OBJ_MAX; i++)
			if (deallocator.segcount[i])
				sciprintf("\t%d\t* %s\n",
					  deallocator.segcount[i],
					  deallocator.segnames[i]);
	}
#endif
}
