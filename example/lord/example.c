#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <SDL.h>
#include "gl.h"

#if defined(__APPLE__) && defined(__MACH__)
#define __unix__	1
#endif

#if defined(__unix__) || defined(unix)
#include <unistd.h>
#else	/* win32 */
#include <windows.h>
#define usleep(x) Sleep(x)
#endif	/* unix / win32 */

int init(void);
void update_gfx(void);
void clean_up(void);
void draw_cube(float sz);
void parse_cmdline(int argc, char **argv);
void *load_image(const char *fname, int *xsz, int *ysz);

int xsz = 320;
int ysz = 240;
const char *img_fname;
unsigned int tex;
SDL_Surface *fbsurf;
int model = 0;

int auto_rot = 1;
int phong_shading = 0;
float xrot, yrot, zrot;

#define MESH_NVERT	5535
#define MESH_NNORM	29124
#define MESH_NFACE	9708

extern float vertices[MESH_NVERT][3];
extern float normals[MESH_NNORM][3];
extern int triangles[MESH_NFACE][3];

unsigned int start_time;

int main(int argc, char **argv) {
	parse_cmdline(argc, argv);

	if(init() == -1) {
		return EXIT_FAILURE;
	}
	
	for(;;) {
		SDL_Event event;
		if(SDL_PollEvent(&event)) {
			if(event.type == SDL_KEYDOWN) {
				if(event.key.keysym.sym == 27) {
					break;
				}

				switch(event.key.keysym.sym) {
				case 32:
					model = (model + 1) % 2;
					break;

				case 'r':
					auto_rot = !auto_rot;
					break;

				case 'p':
					phong_shading = !phong_shading;
					if(phong_shading) {
						glEnable(GL_PHONG);
					} else {
						glDisable(GL_PHONG);
					}

				case SDLK_LEFT:
					yrot -= 5;
					break;

				case SDLK_RIGHT:
					yrot += 5;
					break;

				case SDLK_UP:
					xrot -= 5;
					break;

				case SDLK_DOWN:
					xrot += 5;
					break;

				default:
					break;
				}
			}
#ifdef GP2X
			if(event.type == SDL_JOYBUTTONDOWN) {
				if(event.jbutton.button == GP2X_VK_FX) {
					break;
				}

				switch(event.jbutton.button) {
				case GP2X_VK_LEFT:
					yrot -= 5;
					break;

				case GP2X_VK_RIGHT:
					yrot += 5;
					break;

				case GP2X_VK_UP:
					xrot -= 5;
					break;

				case GP2X_VK_DOWN:
					xrot += 5;
					break;

				case GP2X_VK_FB:
					model = (model + 1) % 2;
					break;

				case GP2X_VK_FA:
					auto_rot = !auto_rot;
					break;

				case GP2X_VK_FR:
					phong_shading = !phong_shading;
					if(phong_shading) {
						glEnable(GL_PHONG);
					} else {
						glDisable(GL_PHONG);
					}
					break;
				}
			}
#endif	/* GP2X */
		} else {
			update_gfx();
		}
	}

	clean_up();
	return 0;
}


int init(void) {
	float lpos[] = {-500, 500, 500, 1.0};
#ifdef GP2X
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK);
	SDL_JoystickOpen(0);
#else
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
#endif
	if(!(fbsurf = SDL_SetVideoMode(xsz, ysz, 16, SDL_SWSURFACE))) {
		fputs("SDL init failed\n", stderr);
		return -1;
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL);

	fglCreateContext();
	glViewport(0, 0, xsz, ysz);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	/*m3d_enable(GL_PHONG);*/

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	/*
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	m3d_enable(GL_BLEND);
	m3d_blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/

	if(img_fname) {
		void *img;
		int tx, ty;
		
		if(!(img = load_image(img_fname, &tx, &ty))) {
			fprintf(stderr, "failed to load %s\n", img_fname);
			return -1;
		}

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, tx, ty, 0, GL_BGRA, GL_UNSIGNED_BYTE, img);
		free(img);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.33333, 50.0, 1000.0);

	{
		float dif[] = {0.4, 0.5, 0.6, 1.0};
		float spec[] = {1, 1, 1, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0f);
	}

	start_time = SDL_GetTicks();

	return 0;
}

void update_gfx(void) {
	static unsigned int frames;
	int i, j;
	uint16_t *pixels;
	float t = (float)(SDL_GetTicks() - start_time) / 20.0f;

	assert(glGetError() == GL_NO_ERROR);

	glClearColor(0.2, 0.2, 0.2, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -500);

	/*if(auto_rot) {
		if(model == 1) glRotatef(t, 1, 0, 0);
		glRotatef(t, 0, 1, 0);
	} else {
		glRotateEulerf(xrot, yrot, zrot);
	}*/

	/*
	
	if(model == 0) {
		glFrontFace(GL_CW);
		glBegin(GL_TRIANGLES);
		for(i=0; i<MESH_NFACE; i++) {
			for(j=0; j<3; j++) {
				int nidx = i * 3 + j;
				int vidx = triangles[i][j];
			
				glNormal3f(normals[nidx][0], normals[nidx][1], normals[nidx][2]);
				glVertex3f(vertices[vidx][0], vertices[vidx][1], vertices[vidx][2]);
			}
		}
		glEnd();

		glFrontFace(GL_CCW);
	} else {
		draw_cube(200);
	}
	*/

	/*glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);
	glVertex3f(-1, 0, 0);
	glVertex3f(1, 0, 0);
	glVertex3f(0, 1, 0);
	glEnd();*/

	glBegin(GL_TRIANGLES);
	glNormal3f(0, 0, 1);
	glVertex3f(0, 100, 0);
	glVertex3f(-100, -100, 0);
	glVertex3f(100, -100, 0);
	glEnd();


	pixels = fglGetFrameBuffer();

	SDL_LockSurface(fbsurf);
	memcpy(fbsurf->pixels, pixels, xsz * ysz * 2);
	SDL_UnlockSurface(fbsurf);
	
	SDL_Flip(fbsurf);
	
	if(frames++ == 20) {
		fprintf(stderr, "20 frames: %u msec\n", SDL_GetTicks() - start_time);
	}
}

void clean_up(void) {
	fglDestroyContext();
	SDL_Quit();
}

void draw_cube(float sz) {
	sz *= 0.5;
	glBegin(GL_QUADS);
	
	/* face +Z */
	glNormal3f(0, 0, 1);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(sz, sz, sz);
	glTexCoord2f(1, 1); glVertex3f(-sz, sz, sz);
	glTexCoord2f(1, 0); glVertex3f(-sz, -sz, sz);
	glTexCoord2f(0, 0); glVertex3f(sz, -sz, sz);

	/* face -X */
	glNormal3f(-1, 0, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 1); glVertex3f(-sz, sz, sz);
	glTexCoord2f(1, 1); glVertex3f(-sz, sz, -sz);
	glTexCoord2f(1, 0); glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(0, 0); glVertex3f(-sz, -sz, sz);

	/* face -Z */
	glNormal3f(0, 0, -1);
	glColor3f(1, 0, 0);
	glTexCoord2f(1, 1); glVertex3f(sz, sz, -sz);
	glTexCoord2f(1, 0); glVertex3f(sz, -sz, -sz);
	glTexCoord2f(0, 0); glVertex3f(-sz, -sz, -sz);
	glTexCoord2f(0, 1); glVertex3f(-sz, sz, -sz);

	/* face +X */
	glNormal3f(1, 0, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(1, 1); glVertex3f(sz, sz, sz);
	glTexCoord2f(1, 0); glVertex3f(sz, -sz, sz);
	glTexCoord2f(0, 0); glVertex3f(sz, -sz, -sz);
	glTexCoord2f(0, 1); glVertex3f(sz, sz, -sz);

	/* face +Y */
	glNormal3f(0, 1, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(1, 1); glVertex3f(sz, sz, -sz);
	glTexCoord2f(0, 1); glVertex3f(-sz, sz, -sz);
	glTexCoord2f(0, 0); glVertex3f(-sz, sz, sz);
	glTexCoord2f(1, 0); glVertex3f(sz, sz, sz);

	/* face -Y */
	glNormal3f(0, -1, 0);
	glColor3f(1, 0, 0);
	glTexCoord2f(0, 0); glVertex3f(sz, -sz, -sz);
	glTexCoord2f(0, 1); glVertex3f(sz, -sz, sz);
	glTexCoord2f(1, 1); glVertex3f(-sz, -sz, sz);
	glTexCoord2f(1, 0); glVertex3f(-sz, -sz, -sz);

	glEnd();
}

void parse_cmdline(int argc, char **argv) {
	int i;
	char *sep;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 's':
				if(!isdigit(argv[++i][0]) || !(sep = strchr(argv[i], 'x')) || !isdigit(*(sep+1))) {
					fprintf(stderr, "malformed -s argument: \"%s\"\n", argv[i]);
					exit(EXIT_FAILURE);
				}
				xsz = atoi(argv[i]);
				ysz = atoi(sep + 1);
				break;

			case 't':
				img_fname = argv[++i];
				break;

			case 'h':
			default:
				fprintf(stderr, "usage: example [-s WxH] [-t tex_fname]\n");
				exit(argv[i][1] == 'h' ? 0 : EXIT_FAILURE);
			}
		} else {
			fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
			exit(EXIT_FAILURE);
		}
	}
}
