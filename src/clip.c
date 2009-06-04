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
#include <math.h>
#include <alloca.h>
#include "clip.h"
#include "vmath.h"

struct plane {
	fixed a, b, c, d;
};

typedef struct vec3f {
	float x, y, z;
} vec3f;

static int clip(struct vertex *outv, struct vertex *inv, int count, int p);
/*static int intersect(vec3 orig, vec3 dir, vec3 pnorm, fixed pdist, fixed *res);*/
static int intersect(vec3f orig, vec3f dir, int p, float *res);
static int inside(struct vec3 pt, int p);
static void interp_vertex(struct vertex *res, struct vertex *v1, struct vertex *v2, fixed t);


#define MAX_CLIP_PLANES		12
#define USER_CLIP_BASE		6

static struct plane cplane[MAX_CLIP_PLANES] = {
	{0, 0, -fixedi(1), -fixedi(1)},	/* near */
	{0, 0, fixedi(1), -fixedi(1)},	/* far */

	{-fixedi(1), 0, 0, -fixedf(1)},	/* right */

	{fixedi(1), 0, 0, -fixedi(1)},	/* left */
	{0, -fixedi(1), 0, -fixedi(1)},	/* top */
	{0, fixedi(1), 0, -fixedi(1)},	/* bottom */

	/* user clipping planes */
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};
static vec3 plane_pos[MAX_CLIP_PLANES] = {
	{0, 0, fixedi(1)},
	{0, 0, -fixedi(1)},
	{fixedf(0.1), 0, 0},
	{-fixedi(1), 0, 0},
	{0, fixedi(1), 0},
	{0, -fixedi(1), 0},

	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0}
};
static int cplane_enable[MAX_CLIP_PLANES] = {
	1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0
};

int enable_clip_plane(int idx)
{
	if(idx < USER_CLIP_BASE || idx >= MAX_CLIP_PLANES) {
		return -1;
	}
	idx += USER_CLIP_BASE;

	cplane_enable[idx] = 1;
	return 0;
}

int disable_clip_plane(int idx)
{
	if(idx < USER_CLIP_BASE || idx >= MAX_CLIP_PLANES) {
		return -1;
	}
	idx += USER_CLIP_BASE;

	cplane_enable[idx] = 0;
	return 0;
}

int set_clip_plane(int idx, fixed a, fixed b, fixed c, fixed d)
{
	if(idx < USER_CLIP_BASE || idx >= MAX_CLIP_PLANES) {
		return -1;
	}
	idx += USER_CLIP_BASE;

	cplane[idx].a = a;
	cplane[idx].b = b;
	cplane[idx].c = c;
	cplane[idx].d = d;

	plane_pos[idx].x = fixed_mul(a, d);
	plane_pos[idx].y = fixed_mul(b, d);
	plane_pos[idx].z = fixed_mul(c, d);
	return 0;
}

int get_clip_plane(int idx, fixed *a, fixed *b, fixed *c, fixed *d)
{
	if(idx < USER_CLIP_BASE || idx >= MAX_CLIP_PLANES) {
		return -1;
	}
	idx += USER_CLIP_BASE;

	*a = cplane[idx].a;
	*b = cplane[idx].b;
	*c = cplane[idx].c;
	*d = cplane[idx].d;
	return 0;
}

int clip_polygon(struct vertex *vert, int count)
{
	int i, vnum, idx = 1;
	struct vertex *vbuf[2];
	fixed w = vert->w;
   
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

		v1.x = vert1->x; v1.y = vert1->y; v1.z = vert1->z; v1.w = vert1->w;
		v2.x = vert2->x; v2.y = vert2->y; v2.z = vert2->z; v2.w = vert2->w;

		if(inside(v2, p)) {
			if(!inside(v1, p)) {
				float t = 0;
				vec3f v1f, dirf;

				v1f.x = fixed_float(v1.x); v1f.y = fixed_float(v1.y); v1f.z = fixed_float(v1.z);
				dirf.x = fixed_float(v2.x) - v1f.x; dirf.y = fixed_float(v2.y) - v1f.y; dirf.z = fixed_float(v2.z) - v1f.z;

				if(intersect(v1f, dirf, p, &t)) {
					printf("t: %f\n", t);
					interp_vertex(outv + ovcount++, vert1, vert2, fixedf(t));
				} else {
					printf("should...\n");
				}
			}
			outv[ovcount++] = *vert2;
		} else {
			if(inside(v1, p)) {
				float t = 0;
				vec3f v1f, dirf;

				v1f.x = fixed_float(v1.x); v1f.y = fixed_float(v1.y); v1f.z = fixed_float(v1.z);
				dirf.x = fixed_float(v2.x) - v1f.x; dirf.y = fixed_float(v2.y) - v1f.y; dirf.z = fixed_float(v2.z) - v1f.z;

				if(intersect(v1f, dirf, p, &t)) {
					printf("t: %f\n", t);
					interp_vertex(outv + ovcount++, vert2, vert1, fixedf(t));
				} else {
					printf("should...\n");
				}

				/*intersect(v1, vm_sub(v2, v1), pnorm, plane->d, &t);
				interp_vertex(outv + ovcount++, vert1, vert2, t);*/
			}
		}
		vert1 = vert2;
	}

	return ovcount;
}

#if 0
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
#endif

static int intersect(vec3f orig, vec3f dir, int p, float *res)
{
	vec3f vo_vec, pnorm;
	float ndotdir, t;

	pnorm.x = fixed_float(cplane[p].a);
	pnorm.y = fixed_float(cplane[p].b);
	pnorm.z = fixed_float(cplane[p].c);

	ndotdir = dir.x * pnorm.x + dir.y * pnorm.y + dir.z * pnorm.z;
	if(fabs(ndotdir) < 0.00001) {
		return 0;
	}

	vo_vec.x = fixed_float(plane_pos[p].x) - orig.x;
	vo_vec.y = fixed_float(plane_pos[p].y) - orig.y;
	vo_vec.z = fixed_float(plane_pos[p].z) - orig.z;

	t = (pnorm.x * vo_vec.x + pnorm.y * vo_vec.y + pnorm.z * vo_vec.z) / ndotdir;
	if(t <= 0.00001 || t > 1.0) {
		return 0;
	}

	*res = t;
	return 1;
}

static int inside(struct vec3 pt, int p)
{
	fixed dot;
	struct vec3 ptov = vm_sub(pt, plane_pos[p]);
	struct vec3 pnorm;

	pnorm.x = cplane[p].a;
	pnorm.y = cplane[p].b;
	pnorm.z = cplane[p].c;

	dot = vm_dot(ptov, pnorm);
	return dot >= 0 ? 1 : 0;
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
