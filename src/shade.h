#ifndef GL_SHADE_H_
#define GL_SHADE_H_

#include "vmath.h"

/* This calculates shading with the phong shading model (half-angle variant)
 * vcs_pos: the surface position in view coordinates
 * vcs_n: the normal in view coordinates
 */
vec3 gl_shade(vec3 vcs_pos, vec3 vcs_n);
void gl_phong_shade(fixed nx, fixed ny, fixed nz, fixed vx, fixed vy, fixed vz, fixed *r, fixed *g, fixed *b, fixed *a);

#endif	/* GL_SHADE_H_ */
