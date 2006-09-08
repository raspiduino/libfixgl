#ifndef GL_VMATH_H_
#define GL_VMATH_H_

#include <stdio.h>
#include "fixed_point.h"

#ifndef HAVE_INLINE
#define inline
#endif	/* HAVE_INLINE */

typedef struct vec3 {
	fixed x, y, z, w;
} vec3;

inline vec3 vm_add(vec3 a, vec3 b);
inline vec3 vm_sub(vec3 a, vec3 b);
inline vec3 vm_mul(vec3 v, fixed s);
inline fixed vm_dot(vec3 a, vec3 b);
inline vec3 vm_normalized(vec3 v);
inline void vm_normalize(vec3 *v);
inline vec3 vm_transform(vec3 v, fixed *m);
void vm_mult_matrix(fixed *t, fixed *m1, fixed *m2);
void vm_print_matrix(FILE *fp, fixed *mat);

#endif	/* GL_VMATH_H_ */
