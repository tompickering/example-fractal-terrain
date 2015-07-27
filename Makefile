all:
	g++ -g main.cpp -o terrain -I/usr/X11R6/include -L/usr/X11R6/lib `sdl-config --libs --cflags` -lGL -lGLU
