
GCC=gcc
GPP=g++

FLAGS=-Wall -g -D DEBUG
#FLAGS=-Wall -g
GCCFLAGS=  -g
CPPFLAGS= -D DEBUG
LIB=

# User definitions must be here
EXEC = Projet.x
INCS =
SOURCES = main.c
OBJS = $(SOURCES:.c=.o)



# Building the world
all: $(EXEC) $(EXEC2)

$(EXEC): $(INCS) $(OBJS)
	$(GCC) $(GCCFLAGS) $(OBJS) $(LIBS) -o $(EXEC)


.SUFFIXES:
.SUFFIXES: .c .cc .o

.cc.o:
	$(GPP) $(FLAGS) -c $<

.c.o:
	$(GCC) $(FLAGS) -c $<


# Clean up
clean:
	rm -f *~ .*~ \#*\# *.o
	rm -f $(EXEC)

# Dependencies
.depend:
	$(GCC) -M $(CPPFLAGS) $(SOURCES) > .depend


include .depend
