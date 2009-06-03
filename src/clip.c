/*
This file is part of libfixgl, a fixed point implementation of OpenGL
Copyright (C) 2006-2009 John Tsiombikas <nuclear@siggraph.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <string.h>
#include <alloca.h>
#include "clip.h"
#include "vmath.h"

struct plane {
	fixed a, b, c, d;
};

static int clip(struct vertex *outv, struct vertex *inv, int count, int p);
static int intersect(vec3 orig, vec3 dir, vec3 pnorm, fixed pdist, fixed *res);
static void interp_vertex(struct vertex *res, struct vertex *v1, struct vertex *v2, fixed t);


#define MAX_CLIP_PLANES		12
static struct plane cplane[MAX_CLIP_PLANES] = {
	{0, 0, -fixedi(1), fixedi(1)},	/* near */
	{0, 0, fixedi(1), -fixedi(1)},	/* far */

	{-fixedi(1), 0, 0, fixedi(0)},	/* right */

	{fixedi(1), 0, 0, -fixedi(1)},	/* left */
	{0, -fixedi(1), 0, fixedi(1)},	/* top */
	{0, fixedi(1), 0, -fixedi(1)},	/* bottom */

	/* user clipping planes */
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};
static int cplane_enable[MAX_CLIP_PLANES] = {
	1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0
};



int clip_polygon(struct vertex *vert, int count)
{
	int i, vnum, idx = 1;
	struct vertex *vbuf[2];
   
	vbuf[0] = alloca(2 * count * sizeof *vbuf[0]);
	vbuf[1] = alloca(2 * count * sizeof *vbuf[1]);

	vnum = clip(vbuf[1], vert, count, 2);
	/*for(i=1; i<MAX_CLIP_PLANES; i++) {
		if(i >= 6 && cplane_enable[i]) {
			vnum = clip(vbuf[(idx + 1) & 1], vbuf[idx & 1], vnum, i);
			idx++;
		}
	}*/

	//memcpy(vert, vbuf[idx & 1], vnum * sizeof *vert);
	memcpy(vert, vbuf[1], vnum * sizeof *vert);
	printf("vnum: %d\n", vnum);
	return vnum;
}


static int clip(struct vertex *outv, struct vertex *inv, int count, int p)
{
	int i, ovcount = 0;
	struct plane *plane = cplane + p;
	vec3 pnorm;
	struct vertex *vert1, *vert2;

	pnorm.x = plane->a;
	pnorm.y = plane->b;
	pnorm.z = plane->c;
	vm_normalize(&pnorm);

	vert1 = inv + count - 1;
	for(i=0; i<count; i++) {
		fixed t;
		vec3 v1, v2;

		vert2 = inv + i;

		v1.x = vert1->x; v1.y = vert1->y; v1.z = vert1->z;
		v2.x = vert2->x; v2.y = vert2->y; v2.z = vert2->z;

		if(vm_dot(v2, pnorm) >= 0) {
			if(vm_dot(v1, pnorm) < 0) {
				intersect(v1, vm_sub(v2, v1), pnorm, plane->d, &t);
				interp_vertex(outv + ovcount++, vert1, vert2, t);
			}
			outv[ovcount++] = *vert1;
		} else {
			if(vm_dot(v1, pnorm) >= 0) {
				intersect(v1, vm_sub(v2, v1), pnorm, plane->d, &t);
				interp_vertex(outv + ovcount++, vert1, vert2, t);
			}
		}
		vert1 = vert2;
	}

	return ovcount;
}

static int intersect(vec3 orig, vec3 dir, vec3 pnorm, fixed pdist, fixed *res)
{
	vec3 ppos, vo_vec;
	fixed ndotdir, t;

	if((ndotdir = vm_dot(dir, pnorm)) == 0) {
		return 0;	/* parallel */
	}

	/* point on plane */
	ppos = vm_mul(pnorm, pdist);
	/* vector from origin to plane */
	vo_vec = vm_sub(ppos, orig);

	t = -fixed_div(vm_dot(pnorm, vo_vec), ndotdir);
	if(t <= 0 || t > fixed_one) {
		return 0;
	}

	*res = t;
	return 1;
}

static void interp_vertex(struct vertex *res, struct vertex *v1, struct vertex *v2, fixed t)
{
	res->x = v1->x + fixed_mul(v2->x - v1->x, t);
	res->y = v1->y + fixed_mul(v2->y - v1->y, t);
	res->z = v1->z + fixed_mul(v2->z - v1->z, t);
	res->w = v1->w + fixed_mul(v2->w - v1->w, t);

	res->r = v1->r + fixed_mul(v2->r - v1->r, t);
	res->g = v1->g + fixed_mul(v2->g - v1->g, t);
	res->b = v1->b + fixed_mul(v2->b - v1->b, t);
	res->a = v1->a + fixed_mul(v2->a - v1->a, t);

	res->u = v1->u + fixed_mul(v2->u - v1->u, t);
	res->v = v1->v + fixed_mul(v2->v - v2->v, t);
}
