#ifndef MainWindow_H
#define MainWindow_H

#include "ui_MainWindow.h"

#include <QMainWindow>
#include <QSettings>

class VisualizationContainer;
class Region;
class RegionTable;

class MainWindow : public QMainWindow, private Ui::MainWindow {
  Q_OBJECT
public:
	MainWindow();
	virtual ~MainWindow();

	void UpdateRegionTable(const std::vector<Region*>& regions);

public slots:
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).

	// Menu events
	virtual void on_actionOpen_Image_File_triggered();
	virtual void on_actionOpen_Image_Stack_triggered();

	virtual void on_actionOpen_Segmentation_File_triggered();
	virtual void on_actionOpen_Segmentation_Stack_triggered();

	virtual void on_actionSave_Segmentation_Data_triggered();

	virtual void on_actionSegment_Volume_triggered();

	virtual void on_actionExit_triggered();

	// Region table events
	virtual void on_removeRegion(int label);

protected:
	// The visualization container
	VisualizationContainer* visualizationContainer;

	// Region table
	RegionTable* regionTable;

	bool eventFilter(QObject* obj, QEvent* event);

	// Default directories
	QString defaultImageDirectoryKey;
	QString defaultSegmentationDirectoryKey;

	QString GetDefaultDirectory(QString key);
	void SetDefaultDirectory(QString key, QString fileName);
};

#endif
