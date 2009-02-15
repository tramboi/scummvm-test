/***************************************************************************
 kmovement.c Copyright (C) 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include <sciresource.h>
#include <engine.h>

/*
Compute "velocity" vector (xStep,yStep)=(vx,vy) for a jump from (0,0) to (dx,dy), with gravity gy. 
The gravity is assumed to be non-negative.

If this was ordinary continuous physics, we would compute the desired (floating point!)
velocity vector (vx,vy) as follows, under the assumption that vx and vy are linearly correlated
by some constant factor c, i.e. vy = c * vx: 
   dx = t * vx
   dy = t * vy + gy * t^2 / 2
=> dy = c * dx + gy * (dx/vx)^2 / 2
=> |vx| = sqrt( gy * dx^2 / (2 * (dy - c * dx)) )
Here, the sign of vx must be chosen equal to the sign of dx, obviously. 

Clearly, this square root only makes sense in our context if the denominator is positive,
or equivalently, (dy - c * dx) must be positive. For simplicity and by symmetry
along the x-axis, we assume dx to be positive for all computations, and only adjust for
its sign in the end. Switching the sign of c appropriately, we set tmp := (dy + c * dx)
and compute c so that this term becomes positive.

Remark #1: If the jump is straight up, i.e. dx == 0, then we should not assume the above
linear correlation vy = c * vx of the velocities (as vx will be 0, but vy shouldn't be,
unless we drop).


Remark #2: We are actually in a discrete setup. The motion is computed iteratively: each iteration,
we add vx and vy to the position, then add gy to vy. So the real formula is the following
(where t is ideally close to an int):

  dx = t * vx
  dy = t * vy + gy * t*(t-1) / 2

But the solution resulting from that is a lot more complicated, so we use the above approximation instead.

Still, what we compute in the end is of course not a real velocity anymore, but an integer approximation,
used in an iterative stepping algorithm
*/
reg_t
kSetJump(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	// Input data
	reg_t object = argv[0];
	int dx = SKPV(1);
	int dy = SKPV(2);
	int gy = SKPV(3);

	// Derived data
	int c;
	int tmp;
	int vx = 0;  // x velocity
	int vy = 0;  // y velocity

	int dxWasNegative = (dx < 0);
	dx = abs(dx);

	assert(gy >= 0);

	if (dx == 0) {
		// Upward jump. Value of c doesn't really matter
		c = 1;
	} else {
		// Compute a suitable value for c respectively tmp.
		// The important thing to consider here is that we want the resulting
		// *discrete* x/y velocities to be not-too-big integers, for a smooth
		// curve (i.e. we could just set vx=dx, vy=dy, and be done, but that
		// is hardly what you would call a parabolic jump, would ya? ;-).
		//
		// So, we make sure that 2.0*tmp will be bigger than dx (that way,
		// we ensure vx will be less than sqrt(gy * dx)).
		if (dx + dy < 0) {
			// dy is negative and |dy| > |dx|
			c = (2*abs(dy)) / dx;
			//tmp = abs(dy);  // ALMOST the resulting value, except for obvious rounding issues
		} else {
			// dy is either positive, or |dy| <= |dx|
			c = (dx*3/2 - dy) / dx;

			// We force c to be strictly positive
			if (c < 1)
				c = 1;

			//tmp = dx*3/2;  // ALMOST the resulting value, except for obvious rounding issues
			
			// FIXME: Where is the 3 coming from? Maybe they hard/coded, by "accident", that usually gy=3 ?
			//  Then this choice of will make t equal to roughly sqrt(dx)
		}
	}
	// POST: c >= 1
	tmp = c * dx + dy;
	// POST: (dx != 0)  ==>  abs(tmp) > abs(dx)
	// POST: (dx != 0)  ==>  abs(tmp) ~>=~ abs(dy)


	SCIkdebug(SCIkBRESEN, "c: %d, tmp: %d\n", c, tmp);
	
	// Compute x step
	if (tmp != 0)
		vx = (int)(dx * sqrt(gy / (2.0 * tmp)));
	else
		vx = 0;

	// Restore the left/right direction: dx and vx should have the same sign.
	if (dxWasNegative)
		vx = -vx;
	
	if ((dy < 0) && (vx == 0)) {
		// Special case: If this was a jump (almost) straight upward, i.e. dy < 0 (upward),
		// and vx == 0 (i.e. no horizontal movement, at least not after rounding), then we
		// compute vy directly.
		// For this, we drop the assumption on the linear correlation of vx and vy (obviously).

		// FIXME: This choice of vy makes t roughly (2+sqrt(2))/gy * sqrt(dy);
		// so if gy==3, then t is roughly sqrt(dy)...
		vy = (int)sqrt(gy * abs(2 * dy)) + 1;
	} else {
		// As stated above, the vertical direction is correlated to the horizontal by the
		// (non-zero) factor c.
		// Strictly speaking, we should probably be using the value of vx *before* rounding
		// it to an integer... Ah well
		vy = c * vx;
	}

	// Always force vy to be upwards
	vy = -abs(vy);

	SCIkdebug(SCIkBRESEN, "SetJump for object at "PREG"\n", PRINT_REG(object));
	SCIkdebug(SCIkBRESEN, "xStep: %d, yStep: %d\n", vx, vy);
	
	PUT_SEL32V(object, xStep, vx);
	PUT_SEL32V(object, yStep, vy);
	
	return s->r_acc;
}

#define _K_BRESEN_AXIS_X 0
#define _K_BRESEN_AXIS_Y 1

void
initialize_bresen(state_t *s, int funct_nr, int argc, reg_t *argv, reg_t mover, int step_factor,
		  int deltax, int deltay)
{
	reg_t client = GET_SEL32(mover, client);
	int stepx = GET_SEL32SV(client, xStep) * step_factor;
	int stepy = GET_SEL32SV(client, yStep) * step_factor;
	int numsteps_x = stepx? (abs(deltax) + stepx-1) / stepx : 0;
	int numsteps_y = stepy? (abs(deltay) + stepy-1) / stepy : 0;
	int bdi, i1;
	int numsteps;
	int deltax_step;
	int deltay_step;

	if (numsteps_x > numsteps_y) {
		numsteps = numsteps_x;
		deltax_step = (deltax < 0)? -stepx : stepx;
		deltay_step = numsteps? deltay / numsteps : deltay;
	} else { /* numsteps_x <= numsteps_y */
		numsteps = numsteps_y;
		deltay_step = (deltay < 0)? -stepy : stepy;
		deltax_step = numsteps? deltax / numsteps : deltax;
	}

	/*  if (abs(deltax) > abs(deltay)) {*/ /* Bresenham on y */
	if (numsteps_y < numsteps_x) {

		PUT_SEL32V(mover, b_xAxis, _K_BRESEN_AXIS_Y);
		PUT_SEL32V(mover, b_incr, (deltay < 0)? -1 : 1);
		/*
		i1 = 2 * (abs(deltay) - abs(deltay_step * numsteps)) * abs(deltax_step);
		bdi = -abs(deltax);
		*/
		i1 = 2*(abs(deltay) - abs(deltay_step * (numsteps - 1))) * abs(deltax_step);
		bdi = -abs(deltax);

	} else { /* Bresenham on x */

		PUT_SEL32V(mover, b_xAxis, _K_BRESEN_AXIS_X);
		PUT_SEL32V(mover, b_incr, (deltax < 0)? -1 : 1);
		/*
		i1= 2 * (abs(deltax) - abs(deltax_step * numsteps)) * abs(deltay_step);
		bdi = -abs(deltay);
		*/
		i1 = 2*(abs(deltax) - abs(deltax_step * (numsteps - 1))) * abs(deltay_step);
		bdi = -abs(deltay);

	}

	PUT_SEL32V(mover, dx, deltax_step);
	PUT_SEL32V(mover, dy, deltay_step);

	SCIkdebug(SCIkBRESEN, "Init bresen for mover "PREG": d=(%d,%d)\n", PRINT_REG(mover), deltax, deltay);
	SCIkdebug(SCIkBRESEN, "    steps=%d, mv=(%d, %d), i1= %d, i2=%d\n",
		  numsteps, deltax_step, deltay_step, i1, bdi*2);

/*	PUT_SEL32V(mover, b_movCnt, numsteps); *//* Needed for HQ1/Ogre? */
	PUT_SEL32V(mover, b_di, bdi);
	PUT_SEL32V(mover, b_i1, i1);
	PUT_SEL32V(mover, b_i2, bdi * 2);

}

reg_t
kInitBresen(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	reg_t mover = argv[0];
	reg_t client = GET_SEL32(mover, client);

	int deltax = GET_SEL32SV(mover, x) - GET_SEL32SV(client, x);
	int deltay = GET_SEL32SV(mover, y) - GET_SEL32SV(client, y);

	initialize_bresen(s, funct_nr, argc, argv, mover, KP_UINT(KP_ALT(1, make_reg(0, 1))), deltax, deltay);

	return s->r_acc;
}


#define MOVING_ON_X (((axis == _K_BRESEN_AXIS_X)&&bi1) || dx)
#define MOVING_ON_Y (((axis == _K_BRESEN_AXIS_Y)&&bi1) || dy)

static enum {
	IGNORE_MOVECNT,
	INCREMENT_MOVECNT,
	UNINITIALIZED
} handle_movecnt = UNINITIALIZED;
	
int parse_reg_t(state_t *s, char *str, reg_t *dest); /* In scriptconsole.c */

static int 
checksum_bytes(byte *data, int size)
{
	int result = 0;
	int i;

	for (i = 0; i < size; i++)
	{
		result += *data;
		data++;
	}

	return result;
}

static void
bresenham_autodetect(state_t *s)
{
	reg_t motion_class;

	if (!parse_reg_t(s, "?Motion", &motion_class))
	{
		object_t *obj = obj_get(s, motion_class);
		reg_t fptr;
		byte *buf;
		
		if (obj == NULL)
		{
			SCIkwarn(SCIkWARNING,"bresenham_autodetect failed!");
			handle_movecnt = INCREMENT_MOVECNT; /* Most games do this, so best guess */
			return;
		}
		
		if (lookup_selector(s, motion_class, s->selector_map.doit, NULL, &fptr) != SELECTOR_METHOD)
		{
			SCIkwarn(SCIkWARNING,"bresenham_autodetect failed!");
			handle_movecnt = INCREMENT_MOVECNT; /* Most games do this, so best guess */
			return;
		}

		buf = s->seg_manager.heap[fptr.segment]->data.script.buf + fptr.offset;
		handle_movecnt = (SCI_VERSION_MAJOR(s->version) == 0 ||
				  checksum_bytes(buf, 8) == 0x216) ? INCREMENT_MOVECNT : IGNORE_MOVECNT;
		sciprintf("b-moveCnt action based on checksum: %s\n", handle_movecnt == IGNORE_MOVECNT ?
			  "ignore" : "increment");
	} else
	{
		SCIkwarn(SCIkWARNING,"bresenham_autodetect failed!");
		handle_movecnt = INCREMENT_MOVECNT; /* Most games do this, so best guess */
	}
}

reg_t
kDoBresen(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	reg_t mover = argv[0];
	reg_t client = GET_SEL32(mover, client);

	int x = GET_SEL32SV(client, x);
	int y = GET_SEL32SV(client, y);
	int oldx, oldy, destx, desty, dx, dy, bdi, bi1, bi2, movcnt, bdelta, axis;
	word signal = GET_SEL32V(client, signal);
	int completed = 0;
	int max_movcnt = GET_SEL32V(client, moveSpeed);

	if (SCI_VERSION_MAJOR(s->version)>0)
		signal&=~_K_VIEW_SIG_FLAG_HIT_OBSTACLE;

	if (handle_movecnt == UNINITIALIZED)
		bresenham_autodetect(s);

	PUT_SEL32(client, signal, make_reg(0, signal)); /* This is a NOP for SCI0 */
	oldx = x;
	oldy = y;
	destx = GET_SEL32SV(mover, x);
	desty = GET_SEL32SV(mover, y);
	dx = GET_SEL32SV(mover, dx);
	dy = GET_SEL32SV(mover, dy);
	bdi = GET_SEL32SV(mover, b_di);
	bi1 = GET_SEL32SV(mover, b_i1);
	bi2 = GET_SEL32SV(mover, b_i2);
	movcnt = GET_SEL32V(mover, b_movCnt);
	bdelta = GET_SEL32SV(mover, b_incr);
	axis = GET_SEL32SV(mover, b_xAxis);

//	sciprintf("movecnt %d, move speed %d\n", movcnt, max_movcnt);

	if (handle_movecnt)
	{
		if (max_movcnt > movcnt)
		{
			++movcnt;
			PUT_SEL32V(mover, b_movCnt, movcnt); /* Needed for HQ1/Ogre? */
			return NULL_REG;
		}
		else
		{
			movcnt = 0;
			PUT_SEL32V(mover, b_movCnt, movcnt); /* Needed for HQ1/Ogre? */
		}
	}

	if ((bdi += bi1) > 0) {
		bdi += bi2;

		if (axis == _K_BRESEN_AXIS_X)
			dx += bdelta;
		else
			dy += bdelta;
	}

	PUT_SEL32V(mover, b_di, bdi);

	x += dx;
	y += dy;

 	if ((MOVING_ON_X
	     && (((x < destx) && (oldx >= destx)) /* Moving left, exceeded? */
		 ||
		 ((x > destx) && (oldx <= destx)) /* Moving right, exceeded? */
		 ||
		 ((x == destx) && (abs(dx) > abs(dy))) /* Moving fast, reached? */
		 /* Treat this last case specially- when doing sub-pixel movements
		 ** on the other axis, we could still be far away from the destination  */
		 )
	     )
	    || (MOVING_ON_Y
		&& (((y < desty) && (oldy >= desty)) /* Moving upwards, exceeded? */
		    ||
		    ((y > desty) && (oldy <= desty)) /* Moving downwards, exceeded? */
		    ||
		    ((y == desty) && (abs(dy) >= abs(dx))) /* Moving fast, reached? */
		    )
		)
	    )
		/* Whew... in short: If we have reached or passed our target position */
		{
			x = destx;
			y = desty;
			completed = 1;

			SCIkdebug(SCIkBRESEN, "Finished mover "PREG"\n", PRINT_REG(mover));
		}


	PUT_SEL32V(client, x, x);
	PUT_SEL32V(client, y, y);

	SCIkdebug(SCIkBRESEN, "New data: (x,y)=(%d,%d), di=%d\n", x, y, bdi);

	if (s->version >= SCI_VERSION_FTU_INVERSE_CANBEHERE)
		invoke_selector(INV_SEL(client, cantBeHere, 0), 0); else
			invoke_selector(INV_SEL(client, canBeHere, 0), 0);

	s->r_acc = not_register(s, s->r_acc);

	if (!s->r_acc.offset) { /* Contains the return value */

		signal = GET_SEL32V(client, signal);

		PUT_SEL32V(client, x, oldx);
		PUT_SEL32V(client, y, oldy);

		PUT_SEL32V(client, signal, (signal | _K_VIEW_SIG_FLAG_HIT_OBSTACLE));

		SCIkdebug(SCIkBRESEN, "Finished mover "PREG" by collision\n", PRINT_REG(mover));
		completed = 1;
	}

	if (SCI_VERSION_MAJOR(s->version)>0)
		if (completed)
			invoke_selector(INV_SEL(mover, moveDone, 0), 0);

	return make_reg(0, completed);
}

extern void
_k_dirloop(reg_t obj, word angle, state_t *s, int funct_nr,
	   int argc, reg_t *argv);
/* From kgraphics.c, used as alternative looper */

int
is_heap_object(state_t *s, reg_t pos);
/* From kscripts.c */

extern int
get_angle(int xrel, int yrel);
/* from kmath.c, used for calculating angles */


reg_t
kDoAvoider(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	reg_t avoider = argv[0];
	reg_t client, looper, mover;
	int angle;
	int dx, dy;
	int destx, desty;


	s->r_acc = make_reg(0, -1);

	if (!is_heap_object(s, avoider)) {
		SCIkwarn(SCIkWARNING, "DoAvoider() where avoider "PREG" is not an object\n", PRINT_REG(avoider));
		return NULL_REG;
	}

	client = GET_SEL32(avoider, client);

	if (!is_heap_object(s, client)) {
		SCIkwarn(SCIkWARNING, "DoAvoider() where client "PREG" is not an object\n", PRINT_REG(client));
		return NULL_REG;
	}

	looper = GET_SEL32(client, looper);

	mover = GET_SEL32(client, mover);

	if (!is_heap_object(s, mover)) {
		if (mover.segment) {
			SCIkwarn(SCIkWARNING, "DoAvoider() where mover "PREG" is not an object\n", PRINT_REG(mover));
		}
		return s->r_acc;
	}

	destx = GET_SEL32V(mover, x);
	desty = GET_SEL32V(mover, y);

	SCIkdebug(SCIkBRESEN, "Doing avoider %04x (dest=%d,%d)\n", avoider, destx, desty);

	if (invoke_selector(INV_SEL(mover, doit, 1) , 0)) {
		SCIkwarn(SCIkERROR, "Mover "PREG" of avoider "PREG
			 " doesn't have a doit() funcselector\n", 
			 PRINT_REG(mover), PRINT_REG(avoider));
		return NULL_REG;
	}

	mover = GET_SEL32(client, mover);
	if (!mover.segment) /* Mover has been disposed? */
		return s->r_acc; /* Return gracefully. */

	if (invoke_selector(INV_SEL(client, isBlocked, 1) , 0)) {
		SCIkwarn(SCIkERROR, "Client "PREG" of avoider "PREG" doesn't"
			 " have an isBlocked() funcselector\n", PRINT_REG(client), PRINT_REG(avoider));
		return NULL_REG;
	}

	dx = destx - GET_SEL32V(client, x);
	dy = desty - GET_SEL32V(client, y);
	angle = get_angle(dx, dy);

	SCIkdebug(SCIkBRESEN, "Movement (%d,%d), angle %d is %sblocked\n",
		  dx, dy, angle, (s->r_acc.offset)? " ": "not ");

	if (s->r_acc.offset) { /* isBlocked() returned non-zero */
		int rotation = (rand() & 1)? 45 : (360-45); /* Clockwise/counterclockwise */
		int oldx = GET_SEL32V(client, x);
		int oldy = GET_SEL32V(client, y);
		int xstep = GET_SEL32V(client, xStep);
		int ystep = GET_SEL32V(client, yStep);
		int moves;

		SCIkdebug(SCIkBRESEN, " avoider "PREG"\n", PRINT_REG(avoider));

		for (moves = 0; moves < 8; moves++) {
			int move_x = (int) (sin(angle * PI / 180.0) * (xstep));
			int move_y = (int) (-cos(angle * PI / 180.0) * (ystep));

			PUT_SEL32V(client, x, oldx + move_x);
			PUT_SEL32V(client, y, oldy + move_y);

			SCIkdebug(SCIkBRESEN, "Pos (%d,%d): Trying angle %d; delta=(%d,%d)\n",
				  oldx, oldy, angle, move_x, move_y);

			if (invoke_selector(INV_SEL(client, canBeHere, 1) , 0)) {
				SCIkwarn(SCIkERROR, "Client "PREG" of avoider "PREG" doesn't"
					 " have a canBeHere() funcselector\n", 
					 PRINT_REG(client), PRINT_REG(avoider));
				return NULL_REG;
			}

			PUT_SEL32V(client, x, oldx);
			PUT_SEL32V(client, y, oldy);

			if (s->r_acc.offset) { /* We can be here */
				SCIkdebug(SCIkBRESEN, "Success\n");
				PUT_SEL32V(client, heading, angle);

				return make_reg(0, angle);
			}

			angle += rotation;

			if (angle > 360)
				angle -= 360;
		}

		SCIkwarn(SCIkWARNING, "DoAvoider failed for avoider "PREG"\n",
			 PRINT_REG(avoider));

	} else {
		int heading = GET_SEL32V(client, heading);

		if (heading == -1)
			return s->r_acc; /* No change */

		PUT_SEL32V(client, heading, angle);

		s->r_acc = make_reg(0, angle);

		if (looper.segment) {
			if (invoke_selector(INV_SEL(looper, doit, 1), 2, angle, client)) {
				SCIkwarn(SCIkERROR, "Looper "PREG" of avoider "PREG" doesn't"
					 " have a doit() funcselector\n", 
					 PRINT_REG(looper), PRINT_REG(avoider));
			} else return s->r_acc;
		} else
		/* No looper? Fall back to DirLoop */

		_k_dirloop(client, (word)angle, s, funct_nr, argc, argv);
	}

	return s->r_acc;
}

