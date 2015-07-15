all:
	g++ main.cpp -o prog -I/usr/X11R6/include -L/usr/X11R6/lib `sdl-config --libs --cflags` -lGL -lGLU
