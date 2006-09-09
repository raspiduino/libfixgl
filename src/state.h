#ifndef GL_STATE_H_
#define GL_STATE_H_

#include "gl_rasterizer.h"
#include "vmath.h"

#define MAX_TEXTURES		128
#define MAX_TEXTURE_SIZE	4096
#define MAX_LIGHTS			4
#define MATRIX_STACK_SIZE	32

enum {MODE_MODELVIEW, MODE_PROJECTION, MODE_TEXTURE};
#define MODE_COUNT		3

#define MAX_POLY		64

struct state {
	/* transformation state */
	unsigned int s;
	int matrix_mode;
	fixed mstack[MODE_COUNT][MATRIX_STACK_SIZE][16];
	int stack_top[MODE_COUNT];
	
	fixed mvp_mat[16];
	int mvp_valid;

	/* texture state */
	struct tex2d *tex[MAX_TEXTURES];
	unsigned int btex;		/* texture bound */

	/* rendering state */
	unsigned int prim;
	struct frame_buffer fb;
	fixed clear_r, clear_g, clear_b, clear_a;
	fixed clear_depth;
	fixed point_sz;

	/* lighting state */
	vec3 lpos[MAX_LIGHTS];
	vec3 ambient, diffuse, specular;
	fixed shininess;

	vec3 ambient_light;

	/* blending state */
	unsigned int src_blend, dst_blend;

	/* misc */
	unsigned int gl_error;
	int in_beg_end;

	/* vertex state */
	fixed r, g, b, a;
	fixed nx, ny, nz;
	fixed tu, tv;
	struct vertex v[MAX_POLY];
	int cur_vert;
	int prim_elem;		/* num elements of the current primitive */
};

#define IS_ENABLED(x)	(state.s & (1 << (x)))

extern struct state state;

#endif	/* GL_STATE_H_ */
