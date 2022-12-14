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

#define LIBFIXGL_SOURCE

#include "gl.h"
#include "state.h"
#include "shade.h"
#include "types.h"
#include "clip.h"

static fixed texgen(int coord, struct vertex *v);

static const int prim_vertices[] = {
	1,		/* GL_POINTS */
	2,		/* GL_LINES */
	-1,		/* GL_LINE_STRIP */
	-1,		/* GL_LINE_LOOP */
	3,		/* GL_TRIANGLES */
	-1,		/* GL_TRIANGLE_STRIP */
	-1,		/* GL_TRIANGLE_FAN */
	4,		/* GL_QUADS */
	-1,		/* GL_QUAD_STRIP */
	-1		/* GL_POLYGON */
};


void glBegin(GLenum primitive)
{
	if(primitive < GL_POINTS || primitive > GL_POLYGON) {
		state.gl_error = GL_INVALID_ENUM;
		return;
	}
	CHECK_BEG_END();

	state.prim = primitive;
	state.cur_vert = 0;
	state.prim_elem = prim_vertices[primitive - GL_POINTS];

	if(!state.mvp_valid) {
		int mvtop = state.stack_top[MODE_MODELVIEW] - 1;
		int ptop = state.stack_top[MODE_PROJECTION] - 1;
		vm_mult_matrix(state.mvp_mat, state.mstack[MODE_PROJECTION][ptop], state.mstack[MODE_MODELVIEW][mvtop]);
		state.mvp_valid = 1;
	}

	state.in_beg_end = 1;
}

void glEnd(void)
{
	if(!state.in_beg_end) {
		state.gl_error = GL_INVALID_OPERATION;
		return;
	}

	if(state.cur_vert) {
		int i;
		struct vertex add_v[2];
		int vcount = state.cur_vert;
		
		switch(state.prim) {
		case GL_LINE_LOOP:
			add_v[0] = state.v[vcount - 1];
			add_v[1] = state.v[0];
			gl_draw_line(add_v);

		case GL_LINE_STRIP:
			for(i=0; i<vcount - 1; i++) {
				gl_draw_line(state.v + i);
			}
			break;

		case GL_POLYGON:
			gl_draw_polygon(state.v, vcount);

		default:
			break;
		}
	}

	state.in_beg_end = 0;
}


void glVertex3x(GLfixed x, GLfixed y, GLfixed z)
{
	fixed *row;
	fixed half_width, half_height;
	vec3 vcs_pos;
	struct vertex *v = state.v + state.cur_vert;
	int mvtop = state.stack_top[MODE_MODELVIEW] - 1;

	v->r = state.r;
	v->g = state.g;
	v->b = state.b;
	v->a = state.a;
	v->nx = state.nx;
	v->ny = state.ny;
	v->nz = state.nz;

	vcs_pos.x = x; vcs_pos.y = y; vcs_pos.z = z;
	vcs_pos = vm_transform(vcs_pos, state.mstack[MODE_MODELVIEW][mvtop]);
	v->vx = vcs_pos.x;
	v->vy = vcs_pos.y;
	v->vz = vcs_pos.z;

	v->u = IS_ENABLED(GL_TEXTURE_GEN_S) ? texgen(GL_S, v) : state.tu;
	v->v = IS_ENABLED(GL_TEXTURE_GEN_T) ? texgen(GL_T, v) : state.tv;
	/*v->w = state.tw;*/
	
	/* if lighting is enabled, modify the color */
	if(IS_ENABLED(GL_LIGHTING)) {
		vec3 normal, col;
		
		if(!(state.s & (1 << GL_PHONG))) {
			normal.x = v->nx;
			normal.y = v->ny;
			normal.z = v->nz;

			col = gl_shade(vcs_pos, normal);
			v->r = col.x;
			v->g = col.y;
			v->b = col.z;
			v->a = col.w;
		}
	}

	/* transform into post-projective homogeneous clip-space */
	row = state.mvp_mat;
	v->x = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3]; row += 4;
	v->y = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3]; row += 4;
	v->z = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3]; row += 4;
	v->w = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z) + row[3];

	printf("--- %.3f %.3f %.3f (%.3f)\n", fixed_float(v->x), fixed_float(v->y), fixed_float(v->z), fixed_float(v->w));

	if(state.prim == GL_POINTS && v->z < 0) {
		return;
	}

	if(++state.cur_vert == state.prim_elem) {
		state.cur_vert = 0;
	}

	if(!state.cur_vert) {
		half_width = fixed_mul(fixedi(state.fb.x), fixed_half);
		half_height = fixed_mul(fixedi(state.fb.y), fixed_half);

		switch(state.prim) {
		case GL_POINTS:
			v->x = fixed_mul(half_width, v->x + fixed_one);
			v->y = fixed_mul(half_height, fixed_one - v->y);
			gl_draw_point(v);
			break;

		case GL_LINES:
			{
				int i;

				for(i=0; i<2; i++) {
					struct vertex *v = state.v + i;
					if(v->w) {
						v->x = fixed_div(v->x, v->w);
						v->y = fixed_div(v->y, v->w);
						v->z = fixed_div(v->z, v->w);
					}
					v->x = fixed_mul(half_width, v->x + fixed_one);
					v->y = fixed_mul(half_height, fixed_one - v->y);
				}
				gl_draw_line(state.v);
			}
			break;

		case GL_TRIANGLES:
		case GL_QUADS:
			{
				int i;
				int vnum = clip_polygon(state.v, state.prim_elem);

				for(i=0; i<vnum; i++) {
					struct vertex *v = state.v + i;

					printf("+++ %.3f %.3f %.3f\n", fixed_float(v->x), fixed_float(v->y), fixed_float(v->z));

					if(v->w) {
						v->x = fixed_div(v->x, v->w);
						v->y = fixed_div(v->y, v->w);
						v->z = fixed_div(v->z, v->w);
						printf(">>> %.3f %.3f\n", fixed_float(v->x), fixed_float(v->y));
					}
					v->x = fixed_mul(half_width, v->x + fixed_one);
					v->y = fixed_mul(half_height, fixed_one - v->y);
				}
				gl_draw_polygon(state.v, vnum);
			}
			break;

		default:
			break;
		}
	}
}

void glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a)
{
	state.r = r;
	state.g = g;
	state.b = b;
	state.a = a;
}

void glNormal3x(GLfixed x, GLfixed y, GLfixed z)
{
	vec3 normal;

	int mvtop = state.stack_top[MODE_MODELVIEW] - 1;
	fixed *row = state.mstack[MODE_MODELVIEW][mvtop];
	
	normal.x = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z); row += 4;
	normal.y = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z); row += 4;
	normal.z = fixed_mul(row[0], x) + fixed_mul(row[1], y) + fixed_mul(row[2], z);

	if(IS_ENABLED(GL_NORMALIZE)) {
		vm_normalize(&normal);
	}
	state.nx = normal.x;
	state.ny = normal.y;
	state.nz = normal.z;
}

void glTexCoord3x(GLfixed s, GLfixed t, GLfixed r)
{
	int ttop = state.stack_top[MODE_TEXTURE] - 1;
	fixed *row = state.mstack[MODE_TEXTURE][ttop];

	state.tu = fixed_mul(row[0], s) + fixed_mul(row[1], t) + fixed_mul(row[2], r); row += 4;
	state.tv = fixed_mul(row[0], s) + fixed_mul(row[1], t) + fixed_mul(row[2], r); row += 4;
	state.tw = fixed_mul(row[0], s) + fixed_mul(row[1], t) + fixed_mul(row[2], r);
}


static fixed texgen(int coord, struct vertex *v)
{
	double sqrt(double);
	int mode = state.tgen[coord - GL_S].mode;
	vec3 u, n;
	static vec3 r;
	static fixed m;
	fixed res, ndotu, fixed_2 = fixedi(2);
	float root, frx, fry, frz;

	switch(mode) {
	case GL_SPHERE_MAP:
		if(coord == GL_S) {
			u.x = 0; u.y = 0; u.z = fixedi(-1);
			n.x = v->nx;
			n.y = v->ny;
			n.z = v->nz;

			ndotu = vm_dot(n, u);
			r.x = u.x - fixed_mul(fixed_mul(fixed_2, n.x), ndotu);
			r.y = u.y - fixed_mul(fixed_mul(fixed_2, n.y), ndotu);
			r.z = u.z - fixed_mul(fixed_mul(fixed_2, n.z), ndotu);

			frx = fixed_float(r.x);
			fry = fixed_float(r.y);
			frz = fixed_float(r.z);

			root = sqrt(frx * frx + fry * fry + (frz + 1.0) * (frz + 1.0));
			m = fixed_mul(fixed_2, fixedf(root));

			res = (m ? fixed_div(r.x, m) : r.x) + fixed_half;
			return res;
		} else {
			return (m ? fixed_div(r.y, m) : r.y) + fixed_half;
		}
		break;

	default:
		break;	/* TODO: implement more modes */
	}

	return 0;
}
