#************************************************************************
#  Makefile for project laserPointer
#************************************************************************
#
# Compiler flags
#
CC=gcc
CFLAGS=-Wall -g
#
# Debug build settings
#
DEBUG=-g
LIBS=-lcheck -lm -lpthread -lrt -lsubunit -lcheck_pic `pkg-config --cflags gtk+-2.0`

all: laserPointer

main-test: main-test.o
	./main-test

main-test.o: main-test.c laserPointer.h
	$(CC) $(CFLAGS) -c main-test.c

laserPointer: main-test.o laserPointer.o
	$(CC) -o laserPointer main-test.o laserPointer.o

laserPointer.o: laserPointer.c laserPointer.h
	$(CC) $(CFLAGS) -c laserPointer.c

lptest: laserPointer-test
	./laserPointer-test

laserPointer-test: laserPointer-test.o laserPointer.o
	$(CC) -o laserPointer-test laserPointer.o laserPointer-test.o $(LIBS)

draw: draw-test
	./draw-test

draw-test: draw-test.o laserPointer.o
	$(CC) -o draw-test laserPointer.o draw-test.o $(LIBS)

## clean != working right now
clean:
	rm main-test main-test.o laserPointer.o laserPointer-test.o draw-test.o
