GLEWPATH = /modules/cs324/glew-1.11.0
INCDIRS= -I$(GLEWPATH)/include 
LIBDIRS= -L/usr/X11R6/lib -L$(GLEWPATH)/lib
LDLIBS = -lglut -lGL -lGLU -lX11 -lm  -lGLEW

CPPFLAGS= $(INCDIRS) -O3 
LDFLAGS= $(CPPFLAGS) $(LIBDIRS) -Wl,-rpath,$(GLEWPATH)/lib

TARGETS = 

SRCS = 

OBJS =  $(SRCS:.cpp=.o)

CXX = g++

DEPS = block.h

default: $(TARGETS)
