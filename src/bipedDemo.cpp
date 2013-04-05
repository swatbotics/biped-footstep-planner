#include "bipedSearch.h"
#include <stdlib.h>
#include <iostream>
#include <vector>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <gl/glut.h>
#endif

#include <math.h>

typedef vec2u<int> vec2i;

int width=0, height=0;
OccupancyGrid<unsigned char> grid;
GLuint grid_texture;

GLint viewport[4];
GLdouble projection[16];
GLdouble modelview[16];

vec2i init(10, 10);
float initTheta = 0;

int goalr = 3;
float inflate_h = 1.0;
float inflate_z = 1.0;

int maxDepth = 1000;
int viewDepth = 40;

vec2i goal(-1,-1);
 
bipedSearch helper;
BipedChecker* checker=0;

biped* searchResult=0;

GLUquadric* quadric = 0;

enum MouseAction {
  MouseNone,
  MouseGoal,
  MouseInit,
  MouseTheta,
};

MouseAction mouse_action = MouseNone;

enum ChangeFlags {
  NoChange          = 0,
  InitChanged      = 0x01,
  GoalChanged      = 0x02,
  InflationChanged = 0x04,
};

int changes = 0;

bool valid(const vec2i& p) {
  return ( p.x() >= 0 && 
           p.y() >= 0 && 
           size_t(p.x()) < grid.nx() && 
           size_t(p.y()) < grid.ny() &&
           grid(p.x(), p.y()) );
}

void updateSearch() { 

  if (!valid(init) || !valid(goal)) { 
    std::cerr << "not running search cause invalid\n";
  }

  if (!checker or (changes & (GoalChanged | InflationChanged))) {
    delete checker;
    checker = new BipedChecker(&grid, goal.x(), goal.y(), inflate_h, inflate_z);
    std::cerr << "built heuristic!\n";
  }
  
  searchResult = helper.search(init.x(), init.y(), initTheta*180/M_PI,
                               goal.x(), goal.y(), goalr,
                               checker, maxDepth, viewDepth);

}

void draw(const biped* b) { 

  if (!b) { return; }

  glPushMatrix();
  glTranslated(b->x, b->y, 0);
  glRotated(b->theta*180/M_PI, 0, 0, 1);

  int r1 = (b->ft == RIGHT) ? 255 : 0;
  int g1 = 0;
  int b1 = (b->ft == LEFT) ? 255 : 0;

  int r0 = r1/4;
  int g0 = g1/4;
  int b0 = b1/4;

  glBegin(GL_QUADS);
  glColor3ub(r1, g1, b1);
  glVertex2f( 2, -1);
  glVertex2f( 2,  1);
  glColor3ub(r0, g0, b0);
  glVertex2f(-2,  1);
  glVertex2f(-2, -1);
  glEnd();
  
  glPopMatrix();
  
}

void display() {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, grid_texture);

  glColor3ub(255,255,255);

  glBegin(GL_QUADS);

  glTexCoord2f(1, 0);
  glVertex2f(grid.nx(), 0);

  glTexCoord2f(0, 0);
  glVertex2f(0, 0);

  glTexCoord2f(0, 1);
  glVertex2f(0, grid.ny());

  glTexCoord2f(1, 1);
  glVertex2f(grid.nx(), grid.ny());

  glEnd();

  glDisable(GL_TEXTURE_2D);

  if (valid(init)) {
    glPushMatrix();

    glColor3ub(63,255,63);
    glTranslated(init.x(), init.y(), 0);
    gluDisk(quadric, 0, goalr, 32, 1);

    glRotated(initTheta*180/M_PI, 0, 0, 1);
    glBegin(GL_TRIANGLES);
    glVertex2f(6, 0);
    glVertex2f(0, 2);
    glVertex2f(0, -2);
    glEnd();

    glPopMatrix();

  }

  if (valid(goal)) {
    glPushMatrix();
    glColor3ub(255,63,63);
    glTranslated(goal.x(), goal.y(), 0);
    gluDisk(quadric, 0, goalr, 32, 1);
    glPopMatrix();
  }

  const biped* b = searchResult;
  while (b) {
    draw(b);
    b = b->pred;
  }

  glutSwapBuffers();

}

void reshape(int w, int h) {

  width = w;
  height = h;

  glViewport(0, 0, width, height);

  float ga = float(grid.nx())/grid.ny();
  float wa = float(w)/h;

  float wh = grid.ny();
  float ww = grid.nx();

  if (ga > wa) {
    wh *= ga/wa;
  } else {
    ww *= wa/ga;
  }

  float cx = 0.5*grid.nx();
  float cy = 0.5*grid.ny();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(cx - 0.5 * ww, cx + 0.5*ww, 
             cy + 0.5 * wh, cy - 0.5*wh);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27: exit(0); break;
  };
}

vec2i unproject(int x, int y) {

  GLdouble ox, oy, oz;
  gluUnProject(x, height-y-1, 0, modelview, projection, viewport, &ox, &oy, &oz);

  return vec2i(ox+0.25, oy+0.25);

}

void motion(int x, int y) {

  vec2i mpos = unproject(x, y);

  if (mouse_action == MouseGoal && valid(mpos) && mpos != goal) {
    
    goal = mpos; 
    changes |= GoalChanged;
    
  } else if (mouse_action == MouseInit && valid(mpos) && mpos != init) {

    init = mpos;
    changes |= InitChanged;

  } else if (mouse_action == MouseTheta && valid(init)) {

    vec2i diff = mpos - init;
    initTheta = atan2(diff.y(), diff.x());
    changes |= InitChanged;

  }

  glutPostRedisplay();
  
}

void mouse(int button, int state, int x, int y) {


  if (mouse_action == MouseNone && state == GLUT_DOWN) {
    
    if (button == GLUT_LEFT_BUTTON) {
      int mod = glutGetModifiers();
      if (mod == GLUT_ACTIVE_SHIFT) {
        mouse_action = MouseInit;
      } else if (mod == GLUT_ACTIVE_CTRL) {
        mouse_action = MouseTheta;
      }
    } else if (button == GLUT_RIGHT_BUTTON) {
      mouse_action = MouseGoal;
    }
    
  }

  if (mouse_action != MouseNone) { 

    motion(x,y); 

    if (state == GLUT_UP) { 

      mouse_action = MouseNone;

      if (changes) { 
        updateSearch();
        glutPostRedisplay();
      }
      
    }

  }

  
}


void usage(int code) {
  std::ostream& ostr = code ? std::cerr : std::cout;
  ostr << "usage: bipedDemo map.png\n";
  exit(code);
}

int main(int argc, char** argv) {

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  glutInitWindowSize(640, 480);

  if (argc < 2) { usage(1); }

  grid.load(argv[1]);
  if (grid.empty()) { 
    std::cerr << "Error loading grid: " << argv[1] << "\n";
    exit(1);
  }
  init = vec2i(8,8);
  goal = vec2i(grid.nx()-8, grid.ny()-8);
  vec2i diff = goal-init;
  initTheta = atan2(diff.y(), diff.x());

  updateSearch();

  glutCreateWindow("Biped plan demo");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  
  glClearColor(1,1,1,1);

  glGenTextures(1, &grid_texture);
  glBindTexture(GL_TEXTURE_2D, grid_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  std::vector<unsigned char> rgbbuf(grid.nx() * grid.ny() * 4);
  int offs = 0;
  for (size_t y=0; y<grid.ny(); ++y) {
    for (size_t x=0; x<grid.nx(); ++x) {
      rgbbuf[offs++] = grid(x,y);
      rgbbuf[offs++] = grid(x,y);
      rgbbuf[offs++] = grid(x,y);
      rgbbuf[offs++] = 255;
    }
  }
    
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               grid.nx(), grid.ny(), 0, GL_RGBA,
               GL_UNSIGNED_BYTE, &(rgbbuf[0]));

  quadric = gluNewQuadric();

  glutMainLoop();

  return 0;

}
