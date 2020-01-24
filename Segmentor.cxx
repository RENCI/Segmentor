#include <QApplication>
#include <QSurfaceFormat>

#include "MainWindow.h"

int main(int argc, char** argv) {
  // Needed to ensure appropriate OpenGL context is created for VTK rendering.
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

  QApplication app(argc, argv);
  app.setApplicationName("Segmentor");
  app.setOrganizationName("RENCI");
  app.setOrganizationDomain("renci.org");

  MainWindow mainWindow;
  mainWindow.show();

  return app.exec();
}
