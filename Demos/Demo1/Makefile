
# Makefile for OpenGL teaching/examples 



## Linux
CC	= gcc
CFLAGS	=-I../util -DGL_GLEXT_PROTOTYPES -Wall
LDFLAGS	= -L/usr/X11R6/lib
GLIBS	= -lglut -lGLU -lGL -lX11 -lXi -lXmu -lm

OBJS = \
	../util/utility.o	\
	../util/glUtils.o	\
	../util/textfile.o

TARGET = shader

$(TARGET): $(TARGET).c $(OBJS)
	/bin/rm -f $@
	$(CC) $(CFLAGS) -o $@ $(TARGET).c $(OBJS)  $(LDFLAGS) $(GLIBS)

../util/utility.o: ../util/utility.c ../util/utility.h
	$(CC) $(CFLAGS) -c -o $@ ../util/utility.c $(GLIBS)

../util/glUtils.o: ../util/glUtils.c ../util/glUtils.h
	$(CC) $(CFLAGS) -c -o $@ ../util/glUtils.c $(GLIBS)

../util/textfile.o: ../util/textfile.c ../util/textfile.h
	$(CC) $(CFLAGS) -c -o $@ ../util/textfile.c $(GLIBS)

clean:
	/bin/rm -f *.o $(TARGET)


