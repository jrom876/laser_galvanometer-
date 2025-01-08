#************************************************************************
#  Makefile file for project laserPointer
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
LIBS=-lcheck -lm -lpthread -lrt -lsubunit -lcheck_pic

DEPS = laserPointer.h

OBJ = laserPointer.o

#*** Targets ***
## % is pattern match
## $@ is whichever target caused the rule's recipe to be run.
## for GNU Make Manual section 10.5.3 Automatic Variables, see:
## https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html#Automatic-Variables

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(LIBS) # the rule's recipe

main: $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

clean:
	rm -f $(OBJ)/*.o
