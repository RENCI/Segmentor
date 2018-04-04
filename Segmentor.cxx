#include <QApplication>
#include <QSurfaceFormat>
#include "MainWindow.h"


int main(int argc, char** argv)
{
  // needed to ensure appropriate OpenGL context is created for VTK rendering.
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

  QApplication app(argc, argv);

  MainWindow MainWindow;
  MainWindow.show();

  return app.exec();
}
