/* This file defines some useful types for cross-platform compatibility. */
#ifndef GL_TYPES_H_
#define GL_TYPES_H_

#include <stdint.h>

/* OpenGL types */
typedef uint8_t 	GLboolean;
typedef int8_t 		GLbyte;
typedef uint8_t 	GLubyte;
typedef int16_t 	GLshort;
typedef uint16_t 	GLushort;
typedef int32_t 	GLint;
typedef uint32_t 	GLuint;
typedef uint32_t 	GLsizei;
typedef uint32_t	GLenum;
typedef uint32_t	GLbitfield;
typedef float		GLfloat;
typedef float		GLclampf;
typedef double		GLdouble;
typedef double		GLclampd;
typedef void		GLvoid;

#define GL_TRUE		1
#define GL_FALSE	0

/* fixed GL types */
#ifdef DBG_USE_FLOAT
typedef float		GLfixed;
typedef float		GLclampx;
#else
typedef int			GLfixed;
typedef int			GLclampx;
#endif


#endif	/* GL_TYPES_H_ */
