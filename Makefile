CC = g++
ifeq ($(shell sw_vers 2>/dev/null | grep Mac | awk '{ print $$2}'),Mac)
	CFLAGS = -g -DGL_GLEXT_PROTOTYPES -I./include/ -I/usr/X11/include -DOSX
	LDFLAGS = -framework GLUT -framework OpenGL \
    	-L"/System/Library/Frameworks/OpenGL.framework/Libraries" \
    	-lGL -lGLU -lm -lstdc++
else
	CFLAGS = -g -DGL_GLEXT_PROTOTYPES -Iglut-3.7.6-bin
	LDFLAGS = -lglut -lGLU
endif
	
RM = /bin/rm -f 
all: main 
main: project_3.o 
	$(CC) -g  $(CFLAGS) -o as3 project_3.o $(LDFLAGS) 
project_3.o: project_3.cpp
	$(CC) -g $(CFLAGS) -I./glm-0.9.4.2 -c project_3.cpp -o project_3.o
clean: 
	$(RM) *.o as3
 


