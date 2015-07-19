#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#define WIDTH  640
#define HEIGHT 480

using std::cout;
using std::endl;
using std::vector;

typedef struct {
  float x;
  float y;
  float z;
  bool is_new;
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

  Point3f p0; p0.x =        0.0; p0.y = BASE_SIZE *  r3 / 2.0; p0.z = 0.0; p0.is_new = false;
  Point3f p1; p1.x = -BASE_SIZE; p1.y = BASE_SIZE * -r3 / 2.0; p1.z = 0.0; p1.is_new = false;
  Point3f p2; p2.x =  BASE_SIZE; p2.y = BASE_SIZE * -r3 / 2.0; p2.z = 0.0; p2.is_new = false;

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
  printf("X %+5.5f, Y %+5.5f, Z %+5.5f, N %d\n", p.x, p.y, p.z, p.is_new);
}

void print_tri() {
  int i;

  printf("TRIANGLE STATE:\n");

  for (i = 0; i < tri.size(); ++i) {
    print_point(tri.at(i));
    if ((i + 1) % 3 == 0) printf("\n");
  }
}

// Calculates 'n' for the triangle, where 'n' is the number of
// points along a side.
int calc_base(int size) {
    int total = 0;
    int base = 0;
    for (int i = 1; total < size; ++i) {
        total += i;
        ++base;
    }
    return base;
}


vector<Point3f> split_triangle(vector<Point3f>* tri) {
  int i;
  int j;

  Point3f p;
  Point3f mp0, mp1, mp2;

  int base = calc_base((*tri).size());
  int nbase = 2*base - 1;

  vector<Point3f> new_points;

  int idx = 0;
  for (i = base; i > 0; --i) {
      // Insert new 'alternating' new-old row of points
      for (j = 0; j < i; j++) {
          // Take 'next' existing point not yet incorporated
          // since the order will be the same in old & new arrays.
          new_points.push_back((*tri).at(idx++));
          if (j+1 != i) {
              Point3f np;
              np.is_new = true;
              new_points.push_back(np);
          }
      }
      // Is an all-new row required?
      // Yes if i-1 != 1
      // Need 2i-2
      if (i > 1) {
          for (j = 0; j < (2*i - 2); ++j) {
              Point3f np;
              np.is_new = true;
              new_points.push_back(np);
          }
      }
  }

  cout << new_points.size() << " new points" << endl;


  // New points inserted
  // Now initialise them
  int offset = 0;
  for (i = nbase; i > 0; --i) {
      for (j = 0; j < i; ++j) {
          Point3f* p = &new_points.at(offset + j);
          if (i % 2) {
              // p might be new, and is in an
              // alternating old-new point row
              if (p->is_new) {
                  Point3f pl = new_points.at(offset + j - 1);
                  Point3f pr = new_points.at(offset + j + 1);
                  p->x = (pl.x + pr.x) / 2.0;
                  p->y = pl.y;
                  p->z = (pl.z + pr.z) / 2.0;
              }
          } else {
              // p is definitely new, as we're in
              // a brand new point row
              Point3f dl = new_points.at(offset + j - i - 1);
              Point3f dr = new_points.at(offset + j - i    );
              p->x = (dl.x + dr.x) / 2.0;
              if (j % 2) {
                  Point3f ul = new_points.at(offset + j + i - 1);
                  p->y = (dr.y + ul.y) / 2.0;
                  p->z = (dr.z + ul.z) / 2.0;
              } else {
                  Point3f ur = new_points.at(offset + j + i    );
                  p->y = (dl.y + ur.y) / 2.0;;
                  p->z = (dl.z + ur.z) / 2.0;;
              }
          }
          p->is_new = false;
      }
      offset += i;
  }

  disp *= down;
  return new_points;
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

  if /*(its < MAX_SUBDIVS)*/ (split) { ++its; split = false; tri = split_triangle(&tri); print_tri();}

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
