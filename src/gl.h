/*
This file is part of libfixgl, a fixed point implementation of OpenGL
Copyright (C) 2006 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef FIXED_GL_H_
#define FIXED_GL_H_

#include "types.h"

enum {
	GL_SMOOTH,
	GL_LIGHTING,
	GL_LIGHT0,
	GL_LIGHT1,
	GL_LIGHT2,
	GL_LIGHT3,
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE,
	GL_DEPTH_TEST,
	GL_DEPTH_WRITE, /* set through glDepthMask() rather than glEnable/glDisable */
	GL_BLEND,
	GL_LIGHT_MODEL_LOCAL_VIEWER,	/* set through glLightModel() */
	GL_LIGHT_MODEL_TWO_SIDE,		/* set through glLightModel() */
	GL_NORMALIZE,
	GL_POINT_SMOOTH,
	GL_LINE_SMOOTH,
	GL_POLYGON_SMOOTH,
	GL_PHONG,	/* oh yeah :) */
	/* -- end of bit fields (acually bit offsets) -- */
	_STATE_BITS_COUNT,

	GL_PROXY_TEXTURE_1D,
	GL_PROXY_TEXTURE_2D,
	GL_PROXY_TEXTURE_3D,
	GL_PROXY_TEXTURE_CUBE,
	GL_FLAT,
	GL_LIGHT_MODEL_AMBIENT,
	GL_LIGHT_MODEL_COLOR_CONTROL,
	GL_SINGLE_COLOR,
	GL_SEPARATE_SPECULAR_COLOR,

	GL_MODELVIEW = 100,
	GL_PROJECTION,
	GL_TEXTURE,

	GL_POINTS = 200,
	GL_LINES,
	GL_LINE_STRIP,
	GL_LINE_LOOP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,	/* XXX: not implemented */
	GL_TRIANGLE_FAN,	/* XXX: not implemented */
	GL_QUADS,
	GL_QUAD_STRIP,		/* XXX: not implemented */
	GL_POLYGON,

	GL_COLOR_BUFFER_BIT = 300,
	GL_DEPTH_BUFFER_BIT,
	GL_ACCUM_BUFFER_BIT,
	GL_STENCIL_BUFFER_BIT,

	GL_POSITION = 400,
	GL_AMBIENT,
	GL_DIFFUSE,
	GL_AMBIENT_AND_DIFFUSE,
	GL_SPECULAR,
	GL_EMISSION,
	GL_SHININESS,
	GL_COLOR_INDEXES,

	GL_SPOT_EXPONENT,
	GL_SPOT_CUTOFF,
	GL_SPOT_DIRECTION,
	GL_CONSTANT_ATTENUATION,
	GL_LINEAR_ATTENUATION,
	GL_QUADRATIC_ATTENUATION,

	GL_FRONT = 450,
	GL_BACK,
	GL_FRONT_AND_BACK,

	GL_ZERO = 500,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,

	GL_RGB = 700,
	GL_RGBA,
	GL_BGR,
	GL_BGRA,
	
	GL_UNSIGNED_BYTE = 800,
	GL_UNSIGNED_SHORT
};

enum {
	GL_NO_ERROR,
	GL_INVALID_ENUM,
	GL_INVALID_VALUE,
	GL_INVALID_OPERATION,
	GL_STACK_OVERFLOW,
	GL_STACK_UNDERFLOW,
	GL_OUT_OF_MEMORY,
	GL_TABLE_TOO_LARGE
};

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void fglCreateContext(void);
void fglDestroyContext(void);
GLuint *fglGetFrameBuffer(void);

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h);

/* clear */
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void glClearColorx(GLfixed r, GLfixed g, GLfixed b, GLfixed a);
void glClearDepth(GLclampd d);
void glClearDepthx(GLfixed d);
void glClear(GLbitfield what);

/* general state */
void glEnable(GLenum what);
void glDisable(GLenum what);
GLboolean glIsEnabled(GLenum what);

void glLightModeli(GLenum pname, GLint val);
void glLightModelf(GLenum pname, GLfloat val);
void glLightModeliv(GLenum pname, GLint *val);
void glLightModelfv(GLenum pname, GLfloat *val);

void glBlendFunc(GLenum src, GLenum dst);
void glShadeModel(GLenum mode);

/* zbuffer state */
void glDepthMask(GLboolean boolval);

/* lights and materials */
void glLightf(GLenum light, GLenum pname, GLfloat param);
void glLightx(GLenum light, GLenum pname, GLfixed param);
void glLighti(GLenum light, GLenum pname, GLint param);
void glLightfv(GLenum light, GLenum pname, GLfloat *params);
void glLightxv(GLenum light, GLenum pname, GLfixed *params);
void glLightiv(GLenum light, GLenum pname, GLint *params);

void glMaterialf(GLenum face, GLenum pname, GLfloat param);
void glMateriali(GLenum face, GLenum pname, GLint param);
void glMaterialx(GLenum face, GLenum pname, GLfixed param);
void glMaterialfv(GLenum face, GLenum pname, GLfloat *params);
void glMaterialiv(GLenum face, GLenum pname, GLint *params);
void glMaterialxv(GLenum face, GLenum pname, GLfixed *params);

/* texture state */
void glGenTextures(GLsizei n, GLuint *tex);
void glDeleteTextures(GLsizei n, const GLuint *tex);
GLboolean glIsTexture(GLuint tex);
void glBindTexture(GLenum targ, GLuint tex);
void glTexImage2D(GLenum targ, GLint lvl, GLint ifmt, GLsizei w, GLsizei h, GLint border, GLenum fmt, GLenum type, const GLvoid *pixels);

/* matrix manipulation */
void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glLoadMatrixf(GLfloat *mat);
void glLoadMatrixx(GLfixed *mat);
void glMultMatrixf(GLfloat *mat);
void glMultMatrixx(GLfixed *mat);

void glPushMatrix(void);
void glPopMatrix(void);

void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glTranslatex(GLfixed x, GLfixed y, GLfixed z);
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
void glRotateEulerf(GLfloat x, GLfloat y, GLfloat z);
void glRotateEulerx(GLfixed x, GLfixed y, GLfixed z);
void glScalef(GLfloat x, GLfloat y, GLfloat z);
void glScalex(GLfixed x, GLfixed y, GLfixed z);

void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat znear, GLfloat zfar);

/* rendering */
void glBegin(GLenum primitive);
void glEnd(void);

void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void glVertex3x(GLfixed x, GLfixed y, GLfixed z);

void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glColor3x(GLfixed r, GLfixed g, GLfixed b);
void glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void glColor4x(GLfixed r, GLfixed g, GLfixed b, GLfixed a);

void glNormal3f(GLfloat x, GLfloat y, GLfloat z);
void glNormal3x(GLfixed x, GLfixed y, GLfixed z);

void glTexCoord2f(GLfloat u, GLfloat v);
void glTexCoord2x(GLfixed u, GLfixed v);


void glPointSize(GLfloat sz);
void glPointSizex(GLfixed sz);

GLenum glGetError(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* FIXED_GL_H_ */
