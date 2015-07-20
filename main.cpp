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


/* The 3D points from which our
 * triangle-based terrain mesh
 * will be built.
 */
typedef struct {
    float x;
    float y;
    float z;
    bool is_new;
} Point3f;


// Parameters for OpenGL rendering
const float BASE_SIZE = 3.5f;
const float DEPTH = -10.f;
const float CAM_PITCH = 265.f;
const float DELTA_ROTATE = 0.8f;


// Subdivision iterations
const int MAX_SUBDIVS = 5;


// Magnitude of vertical vertex
// displacement
float disp = 2.5;


// Damping factor by which 'disp'
// will be multiplied on each
// iteration.
float down = 0.4;


bool split = false;

vector<Point3f> tri;

void init() {
    float r3 = 1.73205;

    glClearColor(0.0, 0.0, 0.2, 1.0); glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45, (double) WIDTH/(double) HEIGHT, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    float dif[] = {1.0, 1.0, 1.0, 1.0};
    float amb[] = {0.2, 0.2, 0.2, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

    Point3f p2; p2.x =        0.0; p2.y = BASE_SIZE *  r3 / 2.0; p2.z = 0.0; p2.is_new = false;
    Point3f p0; p0.x = -BASE_SIZE; p0.y = BASE_SIZE * -r3 / 2.0; p0.z = 0.0; p0.is_new = false;
    Point3f p1; p1.x =  BASE_SIZE; p1.y = BASE_SIZE * -r3 / 2.0; p1.z = 0.0; p1.is_new = false;

    tri.push_back(p0);
    tri.push_back(p1);
    tri.push_back(p2);
}

void init_SDL() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Surface* screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE | SDL_OPENGL);
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


/* Calculates 'n' for the triangle, where 'n'
   is the number of points along a side.
*/
int calc_base(int size) {
    int total = 0;
    int base = 0;
    for (int i = 1; total < size; ++i) {
        total += i;
        ++base;
    }
    return base;
}


/* Take a vector of points representing a
 * triangle-based terrain and operate on it
 * such that each triangle in the mesh is
 * split into four smaller triangles.
 */
void subdivide_terrain(vector<Point3f>* tri) {
    int i;
    int j;

    Point3f p;
    Point3f mp0, mp1, mp2;

    int base = calc_base((*tri).size());
    int nbase = 2*base - 1;

    int idx = 0;
    int ins_idx = 0;

    /* Subdivide the triangular terrain by
     * inserting new points into the vector.
     *
     * For each row of vertices, introduce a
     * new vertex in every odd-indexed position
     * remembering that we're zero-indexing).
     * After this, introduce enough points to
     * consitutea whole new row of vertices in
     * the triangle, of length n-1 (where 'n'
     * is the number of vertices now in the row
     * which we just 'inflated'.
     */
    for (i = base; i > 0; --i) {
        // Insert new points in odd-indexed positions
        for (j = 0; j < i; j++) {
            ins_idx++;
            if (j+1 != i) {
                Point3f np;
                np.is_new = true;
                (*tri).insert((*tri).begin() + ins_idx++, np);
            }
        }
        /* Introduce a new row if we haven't
         * yet reached the tip of the triangle.
         * The number of points needed turns out
         * to be 2i-2.
         */
        if (i > 1) {
            for (j = 0; j < (2*i - 2); ++j) {
                Point3f np;
                np.is_new = true;
                (*tri).insert((*tri).begin() + ins_idx++, np);
            }
        }
    }


    /* New points have now been inserted into the
     * triangular terrain, although they have not
     * yet been initialised with co-ordinates.
     *
     * The triangluar terrain can be thought of
     * as rows of vertices. These rows currently
     * consist of two types; those whose points
     * have all just been added to the triangle,
     * and those which contain some pre-existing
     * ('old') points, in which case they will
     * alternate old-new-old-new...
     *
     * Calculation of each new point's initial
     * co-ordinates depends on which 'type' of
     * row the point belongs to. On an alternating
     * old-new row, all co-ordinates can be
     * derived based on the points either side of
     * it in the row. On completely-new rows
     * however, these values must be calculated
     * based on points in the rows below and above
     * (note that these will always exist, since
     * we never insert * a new row below the base
     * edge or above the tip).
     *
     * Moving from a flat edge to the tip, row 0
     * and all other even-numbered rows will
     * alternate old-new, whilst row 1 and all
     * other odd-numbered rows will consist
     * exclusively of new points.
     */
    int offset = 0;
    for (i = nbase; i > 0; --i) {
        for (j = 0; j < i; ++j) {
            Point3f* p = &(*tri).at(offset + j);
            if (i % 2) {
                // p may or may not be new, as it is in
                // an alternating old-new row of points
                if (p->is_new) {
                    Point3f pl = (*tri).at(offset + j - 1);
                    Point3f pr = (*tri).at(offset + j + 1);
                    p->x = (pl.x + pr.x) / 2.0;
                    p->y = pl.y;
                    p->z = (pl.z + pr.z) / 2.0;
                }
            } else {
                // p is definitely new, as we're in
                // a brand new row of points
                Point3f dl = (*tri).at(offset + j - i - 1);
                Point3f dr = (*tri).at(offset + j - i    );
                p->x = (dl.x + dr.x) / 2.0;
                if (j % 2) {
                    Point3f ul = (*tri).at(offset + j + i - 1);
                    p->y = (dr.y + ul.y) / 2.0;
                    p->z = (dr.z + ul.z) / 2.0;
                } else {
                    Point3f ur = (*tri).at(offset + j + i    );
                    p->y = (dl.y + ur.y) / 2.0;;
                    p->z = (dl.z + ur.z) / 2.0;;
                }
            }
            p->is_new = false;
        }
        offset += i;
    }
}


void distort_terrain(vector<Point3f>* tri) {
    // Vertically offset the vertices
    for (int i = 0; i < (*tri).size(); ++i) {
        (*tri).at(i).z += disp * (float(rand()) / float(RAND_MAX)) - (disp / 2.);
        //(*tri).at(i).y += 0.5 - 0.25 * (float(rand()) / float(RAND_MAX));
        //(*tri).at(i).x += 0.5 - 0.25 * (float(rand()) / float(RAND_MAX));
    }

    disp *= down;
}


/* Simple function to calculate the cross-product
 * of three 3D vectors
 */
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


/* Using OpenGL, render the terrain.
 */
void display(vector<Point3f>* tri) {
    int i, j;
    int idx0, idx1, idx2;

    float pos[] = {-2.0, 2.0, -3.0, 1.0};
    float difamb[] = {1.0, 0.5, 0.3, 1.0};
    float* normal;

    static float angle = 0.f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    glLoadIdentity();

    glTranslatef(-0.5, -2.0, DEPTH);
    glRotatef(CAM_PITCH, 0.1, 0.0, 0.0);
    glRotatef(angle, 0, 0, 1);
    angle += DELTA_ROTATE;

    int base = calc_base((*tri).size());
    int offset = 0;

    glBegin(GL_TRIANGLES);
    for (i = base; i > 0; --i) {
        for (j = 0; j < i - 1; ++j) {
            idx0 = offset + j; idx1 = offset + j + 1; idx2 = offset + j + i;
            normal = cross_product((*tri).at(idx0), (*tri).at(idx1), (*tri).at(idx2));
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, difamb);
            glNormal3f(normal[0], normal[1], normal[2]);

            // Pointy-uppy triangles
            glVertex3f((*tri).at(idx0).x, (*tri).at(idx0).y, (*tri).at(idx0).z);
            glVertex3f((*tri).at(idx1).x, (*tri).at(idx1).y, (*tri).at(idx1).z);
            glVertex3f((*tri).at(idx2).x, (*tri).at(idx2).y, (*tri).at(idx2).z);

            if (j > 0 && (j+1) < i) {
                idx0 = offset + j; idx1 = offset + j + i - 1; idx2 = offset + j + i;
                normal = cross_product((*tri).at(idx0), (*tri).at(idx1), (*tri).at(idx2));
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, difamb);
                glNormal3f(normal[0], normal[1], normal[2]);

                // Pointy-downy triangles
                glVertex3f((*tri).at(idx0).x, (*tri).at(idx0).y, (*tri).at(idx0).z);
                glVertex3f((*tri).at(idx1).x, (*tri).at(idx1).y, (*tri).at(idx1).z);
                glVertex3f((*tri).at(idx2).x, (*tri).at(idx2).y, (*tri).at(idx2).z);
            }
        }
        offset += i;
    }
    glEnd();
    glFlush();
}


int main(void) {
    bool running = true;
    Uint32 start;
    SDL_Event event;

    init_SDL();
    init();

    for (int i = 0; i < MAX_SUBDIVS; ++i) {
        subdivide_terrain(&tri);
        distort_terrain(&tri);
    }

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
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                    }
                    break;
            }
        }
        //if (its < MAX_SUBDIVS || split) {
        //    ++its;
        //    split = false;
        //    subdivide_triangle(&tri);
        //}

        display(&tri);

        SDL_GL_SwapBuffers();

        if (1000 / 60 > (SDL_GetTicks() - start))
            SDL_Delay(1000 / 60 - (SDL_GetTicks() - start));
    }
}
