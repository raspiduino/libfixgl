/* This isn't supposed to be used directly by the user.
 * It's an internal mini3d interface.
 */
#ifndef GL_RASTERIZER_H_
#define GL_RASTERIZER_H_

#include "fixed_point.h"

struct frame_buffer {
	uint32_t *color_buffer;
	uint32_t *depth_buffer;
	/*uint8_t *stencil_buffer;*/
	int x, y;
};

struct tex2d {
	uint32_t *pixels;
	int x, y, pix_count;
	int xpow, xmask, ymask;
	unsigned int type;
};

struct vertex {
	fixed x, y, z, w;
	fixed nx, ny, nz;
	fixed vx, vy, vz;
	fixed r, g, b, a;
	fixed u, v;
};

int gl_rasterizer_setup(struct frame_buffer *fbuf);

void gl_draw_point(struct vertex *pt);
void gl_draw_line(struct vertex *points);
void gl_draw_polygon(struct vertex *points, int count);

#endif	/* GL_RASTERIZER_H_ */
