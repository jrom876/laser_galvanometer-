#************************************************************************
#  Makefile for project laserPointer
#************************************************************************
#
LIBS = `pkg-config --libs gtk+-3.0`

CFLAGS = `pkg-config --cflags gtk+-3.0`

all: test guion

clean:
	rm -f *.o *.so *.html

#### Run 'test' to verify operation only ####
## 		cd ~/laserDriver
## 		make -f make-gui.mk test
clib1.so: clib1.o
	gcc -shared -o libclib1.so clib1.o

clib1.o: clib1.c
	gcc -c -Wall -Werror -fpic clib1.c 

test: clib1.so
	./callclib1.py
####################################################
#### Running 'guion' makes laserPointer.c,
#### opens the python laser GUI, top_gui.py,
#### includes the GTK libraries for later use,
#### and links the two codebases using ctypes shared output files.
## 		cd ~/laserDriver
## 		make -f make-gui.mk guion
laserPointer.so: laserPointer.o
	gcc -shared -o liblaserPointer.so laserPointer.o

laserPointer.o: laserPointer.c
	gcc -c -Wall -Werror -fpic laserPointer.c `pkg-config --cflags gtk+-2.0`

guion: laserPointer.so
	python3 top_gui.py

doc:
	markdown ctypes.md > ctypes.html
	firefox ctypes.html
