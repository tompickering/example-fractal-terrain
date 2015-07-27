#Simple Fractal Terrain Generator

This is a very small program which randomly generates a triangular-mesh-based [fractal terrain](https://en.wikipedia.org/wiki/Fractal_landscape) each time it's run. The mesh is initialised as a single triangle, which is then subdivided into four smaller triangles, after which all vertices are randomly displaced along the vertical axis. Each sub-triangle is then divided further, and the vertices are diplsaced again. This process iterates several times, and the result is a 3D object which could conceivably be used in a video game or 3D modelling project etc.

By default, the program will run 5 iterations of the algorithm and display the result. By passing the '-s' flag to the program, each iteration can be triggered manually while the program is running by pressing 's'.

##Prerequisites
* [SDL](https://www.libsdl.org/) 1.2
* Your favourite OpenGL implementation, such as [Mesa](http://www.mesa3d.org/)

##Note on 'example-' repositories
None of my repositories beginning with 'example-' are intended to be useful pieces of software - rather, they exist to illustrate fun and interesting principles in a complete context. The purpose is simply to show how these sorts of things can be acheived, such that others might be able to deepen their understanding of certain algorithms or the utilities and techniques used to implement them. As such, the files are very thoroughly commented, ocasionally with tangential information such as justifications for design decisions etc. By the same token, I wrote these small programs to develop my own understanding in the first place, so if you can offer any constructuve criticism of my code, or if there's anywhere that I've done anything horifically inefficiently and there is a much simpler and clearer way of writing something then please let me know or throw a pull request my way - it's always greatly appreciated!
