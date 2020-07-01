#ifndef MainWindow_H
#define MainWindow_H

#include "ui_MainWindow.h"

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QSettings>

#include <string>

class VisualizationContainer;
class Region;
class RegionCollection;
class RegionTable;

class MainWindow : public QMainWindow, private Ui::MainWindow {
	Q_OBJECT
public:
	MainWindow();
	virtual ~MainWindow();

	void updateRegions(RegionCollection* regions);
	void updateRegion(Region* region, RegionCollection* regions);
	void selectRegion(unsigned short label);

	void setWindowLevel(double window, double level);

	void setSlicePosition(double x, double y, double z);

	void setVoxelSize(double x, double y, double z);

public slots:

	// Menu events
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).

	virtual void on_actionOpen_Image_File_triggered();
	virtual void on_actionOpen_Image_Stack_triggered();

	virtual void on_actionOpen_Segmentation_File_triggered();
	virtual void on_actionOpen_Segmentation_Stack_triggered();

	virtual void on_actionSave_Image_Data_As_triggered();

	virtual void on_actionSave_Segmentation_Data_triggered();
	virtual void on_actionSave_Segmentation_Data_As_triggered();

	virtual void on_actionSegment_Volume_triggered();

	virtual void on_actionExit_triggered();

	virtual void on_actionBlank_3D_View_triggered(bool checked);

	virtual void on_actionShow_3D_View_triggered(bool checked);
	virtual void on_actionShow_2D_View_triggered(bool checked);
	virtual void on_actionShow_Region_Table_triggered(bool checked);

	// Tool bar events
	virtual void on_actionNavigation();
	virtual void on_actionEdit();
	virtual void on_actionOverlay(bool checked);
	virtual void on_actionVoxels(bool checked);
	virtual void on_actionOutline(bool checked);
	virtual void on_actionRescaleFull(bool checked);
	virtual void on_actionRescalePartial(bool checked);
	virtual void on_actionSmoothNormals(bool checked);
	virtual void on_actionSmoothSurfaces(bool checked);
	virtual void on_actionShowPlane(bool checked);

	// Widget events
	virtual void on_sliceUp();
	virtual void on_sliceDown();

	virtual void on_windowSpinBox_valueChanged(double value);
	virtual void on_levelSpinBox_valueChanged(double value);

	virtual void on_overlaySpinBox_valueChanged(double value);
	virtual void on_overlayUp();
	virtual void on_overlayDown();

	virtual void on_neighborSpinBox_valueChanged(double value);
	virtual void on_neighborUp();
	virtual void on_neighborDown();

	virtual void on_voxelSizeSpinBox();

	virtual void on_brushRadiusSpinBox_valueChanged(int value);
	virtual void on_brushRadiusUp();
	virtual void on_brushRadiusDown();

	// Region table events
	virtual void on_regionDone(int label, bool done);
	virtual void on_removeRegion(int label);
	virtual void on_highlightRegion(int label);
	virtual void on_selectRegion(int label);

protected:
	// The visualization container
	VisualizationContainer* visualizationContainer;

	// Region table
	RegionTable* regionTable;

	void updateLabels(RegionCollection* regions);

	bool eventFilter(QObject* obj, QEvent* event);

	// Default directories
	QString defaultImageDirectoryKey;
	QString defaultSegmentationDirectoryKey;

	QString getDefaultDirectory(QString key);
	void setDefaultDirectory(QString key, QString fileName);

	// Toolbar
	void createToolBar();
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, void (MainWindow::*slot)(bool));
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, bool checked, void (MainWindow::*slot)(bool));
	QLabel* createLabel(const QString& text, int topMargin = 10, int bottomMargin = 5);
};

#endif
