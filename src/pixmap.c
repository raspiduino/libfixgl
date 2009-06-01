#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "pixmap.h"

static int pos_x, pos_y;

static void (*pack[])(void*, int, int, int, int) = {
	pack_pixel32,
	pack_pixel16
};

static void (*unpack[])(const void*, int*, int*, int*, int*) = {
	unpack_pixel32,
	unpack_pixel16
};

static int pixsz[] = { 4, 2 };


void pack_pixel(int fmt, void *pix, int r, int g, int b, int a)
{
	pack[fmt](pix, r, g, b, a);
}

void unpack_pixel(int fmt, const void *pix, int *r, int *g, int *b, int *a)
{
	unpack[fmt](pix, r, g, b, a);
}

void pack_pixel32(void *pix, int r, int g, int b, int a)
{
	*(uint32_t*)pix = PACK_RGBA32(r, g, b, a);
}

void unpack_pixel32(const void *pix, int *r, int *g, int *b, int *a)
{
	UNPACK_RGBA32(*(const uint32_t*)pix, *r, *g, *b, *a);
}

void pack_pixel16(void *pix, int r, int g, int b, int a)
{
	*(uint16_t*)pix = PACK_RGB16(r, g, b);
}

void unpack_pixel16(const void *pix, int *r, int *g, int *b, int *a)
{
	UNPACK_RGB16(*(const uint16_t*)pix, *r, *g, *b);
}

void ras_init_pixmap(struct ras_pixmap *pixmap)
{
	pixmap->width = pixmap->height = pixmap->fmt = -1;
	pixmap->pixels = 0;
	ras_set_pixmap_fmt(pixmap, RAS_RGBA32);
}

void ras_destroy_pixmap(struct ras_pixmap *pixmap)
{
	free(pixmap->pixels);
}

void ras_set_pixmap_fmt(struct ras_pixmap *pixmap, int fmt)
{
	if(pixmap->pixels) {
		void *prev_pix = pixmap->pixels;
		int prev_fmt = pixmap->fmt;
		pixmap->pixels = 0;
		pixmap->fmt = fmt;

		ras_set_pixmap_pixels(pixmap, pixmap->width, pixmap->height, prev_fmt, prev_pix);
		free(prev_pix);
	} else {
		pixmap->fmt = fmt;
	}
	pixmap->bytes_per_pixel = pixsz[fmt];
}

void ras_set_pixmap_pixels(struct ras_pixmap *pixmap, int xsz, int ysz, int sfmt, void *spix)
{
	int i, j;
	unsigned char *dst, *src;

	void *pix = malloc(xsz * ysz * pixsz[sfmt]);
	if(!pix) return;

	dst = pix;
	src = spix;

	for(i=0; i<ysz; i++) {
		for(j=0; j<xsz; j++) {
			int r, g, b, a;
			unpack[sfmt](src, &r, &g, &b, &a);
			pack[pixmap->fmt](dst, r, g, b, a);

			src += pixsz[sfmt];
			dst += pixmap->bytes_per_pixel;
		}
	}

	pixmap->width = xsz;
	pixmap->height = ysz;
	pixmap->bytes_per_line = xsz * pixmap->bytes_per_pixel;

	free(pixmap->pixels);
	pixmap->pixels = pix;
}

void ras_set_pos(int x, int y)
{
	pos_x = x;
	pos_y = y;
}


#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

struct rop_rect {
	int tx, ty;
	int sx, sy;
	int w, h;
};

static int setup_rect(struct rop_rect *rect, int tx, int ty, int tw, int th,
		int x, int y, int w, int h)
{
	int txdif, tydif;

	if(tx >= tw || ty >= th) {
		return -1;
	}

	rect->tx = MAX(tx, 0);
	rect->ty = MAX(ty, 0);

	txdif = rect->tx - tx;
	tydif = rect->ty - ty;

	x += txdif;
	y += tydif;

	w -= txdif;
	h -= tydif;

	if(rect->tx + w >= tw) {
		w = tw - rect->tx;
	}
	if(rect->ty + h >= th) {
		h = th - rect->ty;
	}

	if(w <= 0 || h <= 0) return -1;

	rect->sx = x;
	rect->sy = y;
	rect->w = w;
	rect->h = h;
	return 0;
}

void ras_blit(struct ras_pixmap *dpix, struct ras_pixmap *spix, int x, int y, int w, int h)
{
	int i, j;
	unsigned char *dst, *src;
	struct rop_rect rect;

	if(w < 0) w = spix->width;
	if(h < 0) h = spix->height;

	if(setup_rect(&rect, pos_x, pos_y, dpix->width, dpix->height, x, y, w, h) == -1) {
		return;
	}

	dst = (unsigned char*)dpix->pixels + rect.ty * dpix->bytes_per_line +
		rect.tx * dpix->bytes_per_pixel;
	src = (unsigned char*)spix->pixels + rect.sy * spix->bytes_per_line +
		rect.sx * spix->bytes_per_pixel;

	if(dpix->fmt == spix->fmt) {
		int span = rect.w * dpix->bytes_per_pixel;
		for(i=0; i<rect.h; i++) {
			memcpy(dst, src, span);
			dst += dpix->bytes_per_line;
			src += spix->bytes_per_line;
		}
	} else {
		for(i=0; i<rect.h; i++) {
			unsigned char *s = src;
			unsigned char *d = dst;
			for(j=0; j<rect.w; j++) {
				int r, g, b, a;
				unpack[spix->fmt](s, &r, &g, &b, &a);
				pack[dpix->fmt](d, r, g, b, a);

				d += dpix->bytes_per_pixel;
				s += spix->bytes_per_pixel;
			}
			dst += dpix->bytes_per_line;
			src += spix->bytes_per_line;
		}
	}
}

void ras_fill(struct ras_pixmap *targ, int r, int g, int b, int x, int y, int w, int h)
{
	int i, j;
	struct rop_rect rect;

	if(w < 0) w = targ->width;
	if(h < 0) h = targ->height;

	if(setup_rect(&rect, pos_x, pos_y, targ->width, targ->height, x, y, w, h) == -1) {
		return;
	}

	if(targ->fmt == RAS_RGBA32) {
		uint32_t pixel = PACK_RGBA32(r, g, b, 255);
		uint32_t *dst = (uint32_t*)targ->pixels + rect.ty * targ->width + rect.tx;

		for(i=0; i<rect.h; i++) {
			for(j=0; j<rect.w; j++) {
				dst[j] = pixel;
			}
			dst += targ->width;
		}
	} else {	// 16bpp
		uint16_t pixel = PACK_RGB16(r, g, b);
		uint16_t *dst = (uint16_t*)targ->pixels + rect.ty * targ->width + rect.tx;

		for(i=0; i<rect.h; i++) {
			for(j=0; j<rect.w; j++) {
				dst[j] = pixel;
			}
			dst += targ->width;
		}
	}
}
