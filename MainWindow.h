#ifndef MainWindow_H
#define MainWindow_H

#include "ui_MainWindow.h"

#include <QMainWindow>

class DataPipeline;
class VolumePipeline;
class SlicePipeline;

class MainWindow : public QMainWindow, private Ui::MainWindow {
  Q_OBJECT

public:

	MainWindow();
	virtual ~MainWindow();

public slots:
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).

	// Menu events
	virtual void on_actionOpen_triggered();
	virtual void on_actionExit_triggered();

protected:

	// The visualization pipeline objects
	DataPipeline *dataPipeline;
	VolumePipeline *volumePipeline;
	SlicePipeline *slicePipeline;
};

#endif
