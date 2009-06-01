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
#define LIBFIXGL_SOURCE

#include <stdlib.h>
#include <string.h>
#include "attr.h"

struct attrib_block {
	void **addr;
	void **data;
	int *size;
	int num_attr;

	struct attrib_block *next;
};

static struct attrib_block *atlist;

struct attrib_block *alloc_attrib_block(int atnum)
{
	struct attrib_block *attr;

	if(!(attr = malloc(sizeof *attr))) {
		return 0;
	}
	attr->addr = attr->data = 0;
	attr->size = 0;
	attr->num_attr = 0;

	if(!(attr->addr = malloc(atnum * sizeof *attr->addr))) {
		free_attrib_block(attr);
		return 0;
	}
	if(!(attr->data = malloc(atnum * sizeof *attr->data))) {
		free_attrib_block(attr);
		return 0;
	}
	if(!(attr->size = malloc(atnum * sizeof *attr->size))) {
		free_attrib_block(attr);
		return 0;
	}

	attr->num_attr = 0;
	attr->next = 0;
	return attr;
}

void free_attrib_block(struct attrib_block *attr)
{
	int i;

	if(attr) {
		for(i=0; i<attr->num_attr; i++) {
			free(attr->data[i]);
		}

		free(attr->size);
		free(attr->addr);
		free(attr->data);
		free(attr);
	}
}

int add_attrib(struct attrib_block *attr, void *addr, int size)
{
	void *data;

	if(!(data = malloc(size))) {
		return -1;
	}
	memcpy(data, addr, size);

	attr->addr[attr->num_attr] = addr;
	attr->data[attr->num_attr] = data;
	attr->size[attr->num_attr] = size;
	attr->num_attr++;
	return 0;
}

int push_attrib(struct attrib_block *attr)
{
	attr->next = atlist;
	atlist = attr;
	return 0;
}

int pop_attrib(void)
{
	int i;
	struct attrib_block *attr = atlist;

	if(!attr) {
		return -1;
	}
	atlist = attr->next;

	for(i=0; i<attr->num_attr; i++) {
		memcpy(attr->addr[i], attr->data[i], attr->size[i]);
	}

	free_attrib_block(attr);
	return 0;
}
