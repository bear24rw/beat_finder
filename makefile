# INPUT = USE_FIFO #for fifo input
INPUT = USE_ALSA # for alsa input

CC = gcc
CFLAGS = -Wall -Werror -O2 -std=c99 -lGL -lGLU -lm -lSDL_image -lfftw3 -lftdi -lasound -lpthread `pkg-config ftgl freetype2 --cflags --libs` `sdl-config --cflags --libs` -I/usr/include/freetype2/ -D$(INPUT)
OBJECTS = main.o fft.o draw.o table.o serial.o audio.o

all: compile

debug: CFLAGS += -g -pg
debug: compile

compile : $(OBJECTS)
		$(CC) $(CFLAGS) $(OBJECTS) -o main

%.o : %.c
		$(CC) $(CFLAGS) -c $<

clean:
	@echo Cleaning up...
	rm -fr *.o main
	@echo Done.
