#include <vector>
#include <cstdlib>
#include <cstdio>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#define WIDTH  640
#define HEIGHT 480

using std::vector;

typedef struct {
  float x;
  float y;
  float z;
} Point3f;

const float BASE_SIZE = 3.5f;
const float DEPTH = -10.0f;
const int MAX_SUBDIVS = 5;

float disp = 0.5;
float down = 0.7;

int its = 0;
float angle = 0.0f;
bool split = false;

vector<Point3f> tri;

void init() {
  float r3 = 1.73205;

  glClearColor(0.0, 0.0, 1.0, 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, (double) WIDTH/(double) HEIGHT, 1.0, 500.0);
  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  float dif[] = {1.0, 1.0, 1.0, 1.0};
  float amb[] = {0.2, 0.2, 0.2, 1.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
  glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

  Point3f p0; p0.x =        0.0; p0.y = BASE_SIZE *  r3 / 2.0; p0.z = 0.0;
  Point3f p1; p1.x = -BASE_SIZE; p1.y = BASE_SIZE * -r3 / 2.0; p1.z = 0.0;
  Point3f p2; p2.x =  BASE_SIZE; p2.y = BASE_SIZE * -r3 / 2.0; p2.z = 0.0;

  tri.push_back(p0);
  tri.push_back(p1);
  tri.push_back(p2);
}

void init_SDL() {
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Surface* screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE | SDL_OPENGL);
}

Point3f extract(Point3f* ref) {
  Point3f p;
  p.x = ref->x;
  p.y = ref->y;
  p.z = ref->z;

  return p;
}

void print_point(Point3f p) {
  printf("X %+5.5f, Y %+5.5f, Z %+5.5f\n", p.x, p.y, p.z);
}

void print_tri() {
  int i;

  printf("TRIANGLE STATE:\n");

  for (i = 0; i < tri.size(); ++i) {
    print_point(tri.at(i));
    if ((i + 1) % 3 == 0) printf("\n");
  }
}

vector<Point3f> split_triangle(vector<Point3f>* tri) {
  int i;
  int j;

  Point3f p;
  Point3f mp0, mp1, mp2;
  vector<Point3f> newpoints;
  for (i = 0; i < (*tri).size(); i += 3) {
    mp0.x = ((*tri).at(i).x + (*tri).at(i + 1).x) / 2.0;
    mp0.y = ((*tri).at(i).y + (*tri).at(i + 1).y) / 2.0;
    mp0.z = ((*tri).at(i).z + (*tri).at(i + 1).z) / 2.0 + disp;
    mp1.x = ((*tri).at(i + 1).x + (*tri).at(i + 2).x) / 2.0;
    mp1.y = ((*tri).at(i + 1).y + (*tri).at(i + 2).y) / 2.0;
    mp1.z = ((*tri).at(i + 1).z + (*tri).at(i + 2).z) / 2.0 + disp;
    mp2.x = ((*tri).at(i + 2).x + (*tri).at(i).x) / 2.0;
    mp2.y = ((*tri).at(i + 2).y + (*tri).at(i).y) / 2.0;
    mp2.z = ((*tri).at(i + 2).z + (*tri).at(i).z) / 2.0 + disp;

    newpoints.push_back(extract(&(*tri).at(i)));
    newpoints.push_back(mp0);
    newpoints.push_back(mp2);
    newpoints.push_back(extract(&(*tri).at(i + 1)));
    newpoints.push_back(mp0);
    newpoints.push_back(mp1);
    newpoints.push_back(extract(&(*tri).at(i + 2)));
    newpoints.push_back(mp1);
    newpoints.push_back(mp2);
    newpoints.push_back(mp0);
    newpoints.push_back(mp1);
    newpoints.push_back(mp2);
  }

  disp *= down;
  return newpoints;
}

float* cross_product(Point3f a, Point3f b, Point3f c) {
  float* v = new float[3];

  float* ab = new float[3];
  float* ac = new float[3];
  ab[0] = b.x - a.x; ab[1] = b.y - a.y; ab[2] = b.z - a.z;
  ac[0] = c.x - a.x; ac[1] = c.y - a.y; ac[2] = c.z - a.z;

  v[0] = (ab[1] * ac[2]) - (ab[2] * ac[1]);
  v[1] = (ab[2] * ac[0]) - (ab[0] * ac[2]);
  v[2] = (ab[0] * ac[1]) - (ab[1] * ac[0]);

  return v;
}

void display() {
  int i;

  float pos[] = {-2.0, 2.0, -3.0, 1.0};
  float difamb[] = {1.0, 0.5, 0.3, 1.0};
  float* normal;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  glLoadIdentity();

  glTranslatef(-0.5, -2.0, DEPTH);
  glRotatef(270, 0.1, 0.0, 0.0);
  glRotatef(angle, 0, 0, 1);
  angle += 0.8;

  glBegin(GL_TRIANGLES);
    for (i = 0; i < tri.size(); i += 3) {
      normal = cross_product(tri.at(i), tri.at(i + 1), tri.at(i + 2));
      //glColor3f(rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, difamb);
      glNormal3f(normal[0], normal[1], normal[2]);
      glVertex3f(tri.at(i).x, tri.at(i).y, tri.at(i).z);
      glVertex3f(tri.at(i + 1).x, tri.at(i + 1).y, tri.at(i + 1).z);
      glVertex3f(tri.at(i + 2).x, tri.at(i + 2).y, tri.at(i + 2).z);
    }
  glEnd();
  glFlush();

  if /*(its < MAX_SUBDIVS)*/ (split) { ++its; split = false; tri = split_triangle(&tri); }

  //SDL_Delay(1000);
}

int main(void) {
  bool running = true;
  Uint32 start;
  SDL_Event event;

  init_SDL();
  init();

  while (running) {
    start = SDL_GetTicks();
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
	case SDL_QUIT:
	  running = false;
	  break;
	case SDL_KEYDOWN:
	  switch (event.key.keysym.sym) {
	    case SDLK_s:
	      split = true;
	      break;
	  }
	  break;
      }
    }
    display();
    SDL_GL_SwapBuffers();
    if (1000 / 30 > (SDL_GetTicks() - start)) SDL_Delay(1000 / 30 - (SDL_GetTicks() - start));
  }
}
