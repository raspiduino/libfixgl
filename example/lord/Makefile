obj = example.o lord.o image_png.o
bin = example
lib = ../../libGL.a

SDL_CONFIG = sdl-config

dbg = -g
#opt = -O3 -ffast-math

CC = gcc
CFLAGS = -std=gnu89 -pedantic -Wall $(opt) $(dbg) -I../../src `$(SDL_CONFIG) --cflags` -I/usr/local/devkitPro/devkitGP2X/include/libpng12
LDFLAGS = -lm `$(SDL_CONFIG) --libs` -lpng -lz -lpthread

$(bin): $(obj) $(lib)
	$(CC) -o $@ $(obj) $(lib) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(bin) $(obj)
