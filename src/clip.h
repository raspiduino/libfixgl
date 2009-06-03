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
#ifndef CLIP_H_
#define CLIP_H_

#include "gl_rasterizer.h"

int clip_polygon(struct vertex *vert, int count);

#endif /* CLIP_H_ */
