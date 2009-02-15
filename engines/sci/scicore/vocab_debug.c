/* Modified 07/18/99 by Christoph Reichenbach:
** - added vocabulary_free_snames();
*/


#include <string.h>
#include <engine.h>
#include <sciresource.h>

/* Default kernel name table */
#define SCI0_KNAMES_WELL_DEFINED 0x6e
#define SCI0_KNAMES_DEFAULT_ENTRIES_NR 0x72

const char *sci0_default_knames[SCI0_KNAMES_DEFAULT_ENTRIES_NR] =
{
/*0x00*/ "Load",
/*0x01*/ "UnLoad",
/*0x02*/ "ScriptID",
/*0x03*/ "DisposeScript",
/*0x04*/ "Clone",
/*0x05*/ "DisposeClone",
/*0x06*/ "IsObject",
/*0x07*/ "RespondsTo",
/*0x08*/ "DrawPic",
/*0x09*/ "Show",
/*0x0a*/ "PicNotValid",
/*0x0b*/ "Animate",
/*0x0c*/ "SetNowSeen",
/*0x0d*/ "NumLoops",
/*0x0e*/ "NumCels",
/*0x0f*/ "CelWide",
/*0x10*/ "CelHigh",
/*0x11*/ "DrawCel",
/*0x12*/ "AddToPic",
/*0x13*/ "NewWindow",
/*0x14*/ "GetPort",
/*0x15*/ "SetPort",
/*0x16*/ "DisposeWindow",
/*0x17*/ "DrawControl",
/*0x18*/ "HiliteControl",
/*0x19*/ "EditControl",
/*0x1a*/ "TextSize",
/*0x1b*/ "Display",
/*0x1c*/ "GetEvent",
/*0x1d*/ "GlobalToLocal",
/*0x1e*/ "LocalToGlobal",
/*0x1f*/ "MapKeyToDir",
/*0x20*/ "DrawMenuBar",
/*0x21*/ "MenuSelect",
/*0x22*/ "AddMenu",
/*0x23*/ "DrawStatus",
/*0x24*/ "Parse",
/*0x25*/ "Said",
/*0x26*/ "SetSynonyms",
/*0x27*/ "HaveMouse",
/*0x28*/ "SetCursor",
/*0x29*/ "FOpen",
/*0x2a*/ "FPuts",
/*0x2b*/ "FGets",
/*0x2c*/ "FClose",
/*0x2d*/ "SaveGame",
/*0x2e*/ "RestoreGame",
/*0x2f*/ "RestartGame",
/*0x30*/ "GameIsRestarting",
/*0x31*/ "DoSound",
/*0x32*/ "NewList",
/*0x33*/ "DisposeList",
/*0x34*/ "NewNode",
/*0x35*/ "FirstNode",
/*0x36*/ "LastNode",
/*0x37*/ "EmptyList",
/*0x38*/ "NextNode",
/*0x39*/ "PrevNode",
/*0x3a*/ "NodeValue",
/*0x3b*/ "AddAfter",
/*0x3c*/ "AddToFront",
/*0x3d*/ "AddToEnd",
/*0x3e*/ "FindKey",
/*0x3f*/ "DeleteKey",
/*0x40*/ "Random",
/*0x41*/ "Abs",
/*0x42*/ "Sqrt",
/*0x43*/ "GetAngle",
/*0x44*/ "GetDistance",
/*0x45*/ "Wait",
/*0x46*/ "GetTime",
/*0x47*/ "StrEnd",
/*0x48*/ "StrCat",
/*0x49*/ "StrCmp",
/*0x4a*/ "StrLen",
/*0x4b*/ "StrCpy",
/*0x4c*/ "Format",
/*0x4d*/ "GetFarText",
/*0x4e*/ "ReadNumber",
/*0x4f*/ "BaseSetter",
/*0x50*/ "DirLoop",
/*0x51*/ "CanBeHere",
/*0x52*/ "OnControl",
/*0x53*/ "InitBresen",
/*0x54*/ "DoBresen",
/*0x55*/ "DoAvoider",
/*0x56*/ "SetJump",
/*0x57*/ "SetDebug",
/*0x58*/ "InspectObj",
/*0x59*/ "ShowSends",
/*0x5a*/ "ShowObjs",
/*0x5b*/ "ShowFree",
/*0x5c*/ "MemoryInfo",
/*0x5d*/ "StackUsage",
/*0x5e*/ "Profiler",
/*0x5f*/ "GetMenu",
/*0x60*/ "SetMenu",
/*0x61*/ "GetSaveFiles",
/*0x62*/ "GetCWD",
/*0x63*/ "CheckFreeSpace",
/*0x64*/ "ValidPath",
/*0x65*/ "CoordPri",
/*0x66*/ "StrAt",
/*0x67*/ "DeviceInfo",
/*0x68*/ "GetSaveDir",
/*0x69*/ "CheckSaveGame",
/*0x6a*/ "ShakeScreen",
/*0x6b*/ "FlushResources",
/*0x6c*/ "SinMult",
/*0x6d*/ "CosMult",
/*0x6e*/ "SinDiv",
/*0x6f*/ "CosDiv",
/*0x70*/ "Graph",
/*0x71*/ SCRIPT_UNKNOWN_FUNCTION_STRING
};


int getInt(unsigned char* d)
{
  return d[0] | (d[1]<<8);
}

int* vocabulary_get_classes(resource_mgr_t *resmgr, int* count)
{
  resource_t* r;
  int *c;
  unsigned int i;

  if((r = scir_find_resource(resmgr, sci_vocab, 996, 0)) == NULL) return 0;

  c= (int*)sci_malloc(sizeof(int)*r->size/2);
  for(i=2; i<r->size; i+=4)
    {
      c[i/4]=getInt(r->data+i);
    }
  *count=r->size/4;

  return c;
}

int vocabulary_get_class_count(resource_mgr_t *resmgr)
{
  resource_t* r;
  if((r = scir_find_resource(resmgr, sci_vocab, 996, 0))==0) return 0;
  return r->size/4;
}

char** vocabulary_get_snames(resource_mgr_t *resmgr, int* pcount, sci_version_t version)
{
  char** t;
  int count;
  int i,j;
  int magic;

  resource_t* r = scir_find_resource(resmgr, sci_vocab, 997, 0);

  if (!r) /* No such resource? */
    return NULL;

  count=getInt(r->data) + 1; /* Counter is slightly off */

  magic=((version==0) || (version>=SCI_VERSION_FTU_NEW_SCRIPT_HEADER))? 1 : 2;

  t= (char**)sci_malloc(sizeof(char*)*magic*(count+1));

  j=0;

  for(i=0; i<count; i++)
    {
      int offset=getInt(r->data+2+i*2);
      int len=getInt(r->data+offset);
      t[j]= (char*)sci_malloc(len+1);
      memcpy(t[j], r->data+offset+2, len);
      t[j][len]='\0';
      j++;
      if ((version!=0) && (version<SCI_VERSION_FTU_NEW_SCRIPT_HEADER))
      {
        t[j]= (char*)sci_malloc(len+1);
        memcpy(t[j], r->data+offset+2, len);
        t[j][len]='\0';
        j++;
      }
    }

  t[j]=0;

  if (pcount != NULL) *pcount=magic*count;

  return t;
}

int
vocabulary_lookup_sname(char **snames_list, char *sname)
{
  int pos = 0;
  while (snames_list[pos])
  {
	  if (!strcasecmp(sname, snames_list[pos])) return pos;
	  pos++;
  }

  return -1;
}

void
vocabulary_free_snames(char **snames_list)
{
  int pos = 0;

  while (snames_list[pos])
    free(snames_list[pos++]);

  free(snames_list);
}

opcode* vocabulary_get_opcodes(resource_mgr_t *resmgr)
{
	opcode* o;
	int count, i=0;
	resource_t* r = scir_find_resource(resmgr, sci_vocab, VOCAB_RESOURCE_OPCODES, 0);

	/* if the resource couldn't be loaded, leave */
	if (r == NULL) {
		fprintf(stderr,"unable to load vocab.%03d\n", VOCAB_RESOURCE_OPCODES);
		return NULL;
	}

	count=getInt(r->data);

	o= (opcode*)sci_malloc(sizeof(opcode)*256);
	for(i=0; i<count; i++)
		{
			int offset=getInt(r->data+2+i*2);
			int len=getInt(r->data+offset)-2;
			o[i].type=getInt(r->data+offset+2);
			o[i].number=i;
			o[i].name= (char*)sci_malloc(len+1);
			memcpy(o[i].name, r->data+offset+4, len);
			o[i].name[len]='\0';
#ifdef VOCABULARY_DEBUG
			printf("Opcode %02X: %s, %d\n", i, o[i].name, o[i].type);
#endif
		}
	for(i=count; i<256; i++)
		{
			o[i].type=0;
			o[i].number=i;
			o[i].name= (char*)sci_malloc(strlen("undefined")+1);
			strcpy(o[i].name, "undefined");
		}
	return o;
}

void
vocabulary_free_opcodes(opcode *opcodes)
{
	int i;
	if (!opcodes)
		return;

	for (i = 0; i < 256; i++) {
		if (opcodes[i].name)
			free(opcodes[i].name);
	}
	free(opcodes);
}


/* Alternative kernel func names retriever. Required for KQ1/SCI (at least). */
static char** _vocabulary_get_knames0alt(int *names, resource_t *r)
{
  unsigned int mallocsize = 32;
  char **retval = (char**)sci_malloc(sizeof (char *) * mallocsize);
  unsigned int i = 0, index = 0;

  while (index < r->size) {

    int slen = strlen((char *) r->data + index) + 1;

    retval[i] = (char*)sci_malloc(slen);
    memcpy(retval[i++], r->data + index, slen);
    /* Wouldn't normally read this, but the cleanup code wants to free() this */

    index += slen;

    if (i == mallocsize)
      retval = (char**)sci_realloc(retval, sizeof(char *) * (mallocsize <<= 1));

  }

  *names = i + 1;
  retval = (char**)sci_realloc(retval, sizeof(char *) * (i+2));
  retval[i] = (char*)sci_malloc(strlen(SCRIPT_UNKNOWN_FUNCTION_STRING) + 1);
  strcpy(retval[i], SCRIPT_UNKNOWN_FUNCTION_STRING);
  /* The mystery kernel function- one in each SCI0 package */

  retval[i+1] = NULL; /* Required for cleanup */

  return retval;
}


static char** vocabulary_get_knames0(resource_mgr_t *resmgr, int* names)
{
	char** t;
	int count, i, index=2, empty_to_add = 1;
	resource_t* r = scir_find_resource(resmgr, sci_vocab, VOCAB_RESOURCE_KNAMES, 0);


	if (!r) { /* No kernel name table found? Fall back to default table */
		t = (char**)sci_malloc ((SCI0_KNAMES_DEFAULT_ENTRIES_NR + 1) * sizeof(char*));
		*names = SCI0_KNAMES_DEFAULT_ENTRIES_NR - 1; /* index of last element */

		for (i = 0; i < SCI0_KNAMES_DEFAULT_ENTRIES_NR; i++)
			t[i] = sci_strdup(sci0_default_knames[i]);

		t[SCI0_KNAMES_DEFAULT_ENTRIES_NR] = NULL; /* Terminate list */

		return t;
	}

	count=getInt(r->data);

	if (count > 1023)
		return _vocabulary_get_knames0alt(names, r);

	if (count < SCI0_KNAMES_WELL_DEFINED) {
		empty_to_add = SCI0_KNAMES_WELL_DEFINED - count;
		sciprintf("Less than %d kernel functions; adding %d\n", SCI0_KNAMES_WELL_DEFINED, empty_to_add);
	}

	t= (char**)sci_malloc(sizeof(char*)*(count+1 + empty_to_add));
	for(i=0; i<count; i++) {
		int offset=getInt(r->data+index);
		int len=getInt(r->data+offset);
		/*fprintf(stderr,"Getting name %d of %d...\n", i, count);*/
		index+=2;
		t[i]= (char*)sci_malloc(len+1);
		memcpy(t[i], r->data + offset + 2, len);
		t[i][len]='\0';
	}

	for (i = 0; i < empty_to_add; i++) {
		t[count + i] = (char*)sci_malloc(strlen(SCRIPT_UNKNOWN_FUNCTION_STRING) +1);
		strcpy(t[count + i], SCRIPT_UNKNOWN_FUNCTION_STRING);
	}

	t[count+empty_to_add]=0;
	*names=count + empty_to_add;
	return t;
}

/*NOTE: Untested*/
static char** vocabulary_get_knames1(resource_mgr_t *resmgr, int *count)
{
  char** t=NULL;
  unsigned int size=64, used=0, pos=0;
  resource_t* r = scir_find_resource(resmgr, sci_vocab, VOCAB_RESOURCE_KNAMES, 0);

  while(pos<r->size)
    {
      int len;
      if ((used==size-1)||(!t))
	{
	  size*=2;
	  t= (char**)sci_realloc(t, size*sizeof(char*));
	}
      len=strlen((char *) r->data+pos);
      t[used]= (char*)sci_malloc(len+1);
      strcpy(t[used], (char *) r->data+pos);
      used++;
      pos+=len+1;
    }
  *count=used;
  t= (char**)sci_realloc(t, (used+1)*sizeof(char*));
  t[used]=NULL;
  return t;
}

char** vocabulary_get_knames(resource_mgr_t *resmgr, int* count)
{
	switch(resmgr->sci_version)
	{
		case SCI_VERSION_0:
		case SCI_VERSION_01:
		case SCI_VERSION_01_VGA:
		case SCI_VERSION_01_VGA_ODD: return vocabulary_get_knames0(resmgr, count);
		case SCI_VERSION_1_EARLY:
	        case SCI_VERSION_1_LATE:
	        case SCI_VERSION_1_1:
		case SCI_VERSION_32: return vocabulary_get_knames1(resmgr, count);
		default: return 0;
	}
}

void vocabulary_free_knames(char** names)
{
	int i	= 0;
	while(names[i])
	{
		free(names[i]);
		i++;
	}

	free(names);
}
