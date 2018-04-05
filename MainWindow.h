#ifndef MainWindow_H
#define MainWindow_H

#include "ui_MainWindow.h"

#include <QMainWindow>

class VolumePipeline;
class SlicePipeline;

class MainWindow : public QMainWindow, private Ui::MainWindow {
  Q_OBJECT

public:

  MainWindow();

public slots:

  virtual void slotExit();

protected:

	// The visualization pipeline objects
	VolumePipeline *volumePipeline;
	SlicePipeline *slicePipeline;
};

#endif
