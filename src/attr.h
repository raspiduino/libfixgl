/*
This file is part of libfixgl, a fixed point implementation of OpenGL
Copyright (C) 2006 - 2009 John Tsiombikas <nuclear@siggraph.org>

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
#ifndef GL_ATTR_H_
#define GL_ATTR_H_

#ifndef LIBFIXGL_SOURCE
#error "attr.h should not included by user programs, just include gl.h"
#endif

struct attrib_block;

struct attrib_block *alloc_attrib_block(int atnum);
void free_attrib_block(struct attrib_block *ablock);
int add_attrib(struct attrib_block *ablock, void *addr, int sz);

int push_attrib(struct attrib_block *ablock);
int pop_attrib(void);

#endif	/* GL_ATTR_H_ */
