
# Makefile for OpenGL teaching/examples 



## Linux
CC	= gcc
CFLAGS	= -I../util -DGL_GLEXT_PROTOTYPES -Wall
LDFLAGS	= -L/usr/X11R6/lib
GLIBS	= -lglut -lGLU -lGL -lX11 -lXi -lXmu -lm


OBJS = \
	../util/utility.o	\
	../util/glUtils.o       \
        ../util/textfile.o


TARGET = phong

$(TARGET): $(TARGET).c $(OBJS)
	/bin/rm -f $@
	$(CC) $(CFLAGS) -o $@ $(TARGET).c $(OBJS) $(LDFLAGS) $(GLIBS)

clean:
	/bin/rm -f *.o $(TARGET)

