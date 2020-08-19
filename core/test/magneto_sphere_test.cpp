#include <GL/glut.h>
#include <gtest/gtest.h>
#include <indoors/magnetics/magneto_sphere.h>
#include <thread>

using namespace indoors::magnetics;

MagnetoSphere ms;

TEST(magneto_sphere, hello) {
  int argc = 0;
  char **argv = nullptr;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

  glutInitWindowPosition(200, 100);
  glutInitWindowSize(1000, 800);
  glutCreateWindow("A Simple Triangle");

  ms.surface_created();
  ms.surface_changed(1000, 800);

  glutDisplayFunc([]() { ms.draw_frame(); });

  glutMainLoop();
}
