#ifndef RAS_PIXMAP_H_
#define RAS_PIXMAP_H_

enum {
	RAS_RGBA32,
	RAS_RGB16
};

struct ras_pixmap {
	int width, height;
	int fmt;
	void *pixels;
	int bytes_per_pixel, bytes_per_line;
};

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void ras_init_pixmap(struct ras_pixmap *pixmap);
void ras_destroy_pixmap(struct ras_pixmap *pixmap);

void ras_set_pixmap_fmt(struct ras_pixmap *pixmap, int fmt);
void ras_set_pixmap_pixels(struct ras_pixmap *pixmap, int xsz, int ysz, int sfmt, void *spix);

void ras_set_pos(int x, int y);

#define RAS_FULL_SIZE	(-1)

void ras_blit(struct ras_pixmap *targ, struct ras_pixmap *src, int x, int y, int w, int h);
void ras_fill(struct ras_pixmap *targ, int r, int g, int b, int x, int y, int w, int h);

#define ALPHA_SHIFT32	24
#define RED_SHIFT32		16
#define GREEN_SHIFT32	8
#define BLUE_SHIFT32	0

#define PACK_RGBA32(r, g, b, a)	\
	((((a) & 0xff) << ALPHA_SHIFT32) | \
	 (((r) & 0xff) << RED_SHIFT32) | \
	 (((g) & 0xff) << GREEN_SHIFT32) | \
	 (((b) & 0xff) << BLUE_SHIFT32))

#define UNPACK_RGBA32(p, r, g, b, a)	\
	do {	\
		a = ((p) >> ALPHA_SHIFT32) & 0xff;	\
		r = ((p) >> RED_SHIFT32) & 0xff;	\
		g = ((p) >> GREEN_SHIFT32) & 0xff;	\
		b = ((p) >> BLUE_SHIFT32) & 0xff;	\
	} while(0)

#define PACK_RGB16(r, g, b)	\
	((((r) & 0xf8) << 8) | 	\
	 (((g) & 0xfc) << 3) |	\
	 (((b) & 0xf8) >> 3))

#define UNPACK_RGB16(p, r, g, b)	\
	do {	\
		r = ((p) >> 8) & 0xf8;	\
		g = ((p) >> 3) & 0xfc;	\
		b = ((p) << 3) & 0xf8;	\
	} while(0)

void pack_pixel(int fmt, void *pix, int r, int g, int b, int a);
void unpack_pixel(int fmt, const void *pix, int *r, int *g, int *b, int *a);

void pack_pixel32(void *pix, int r, int g, int b, int a);
void unpack_pixel32(const void *pix, int *r, int *g, int *b, int *a);

void pack_pixel16(void *pix, int r, int g, int b, int a);
void unpack_pixel16(const void *pix, int *r, int *g, int *b, int *a);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* RAS_PIXMAP_H_ */
