CC=g++
OBJS=terrain.o
LIBS=-lSDL -lGL -lGLU
NAME=terrain

all: $(OBJS)
	$(CC) *.o -o $(NAME) $(LIBS)

%.o : $*.cpp $*.hpp
	$(CC) -c $^ -O3 $(LIBS)

clean:
	rm *[.]o
