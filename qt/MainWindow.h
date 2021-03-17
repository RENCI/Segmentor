#ifndef MainWindow_H
#define MainWindow_H

#include "ui_MainWindow.h"

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QProgressDialog>
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

	void setSlicePosition(double x, double y, double z);

	void updateProgress(double progress);

	void showMessage(QString message);

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

	virtual void on_actionUndo_triggered();
	virtual void on_actionRedo_triggered();

	virtual void on_actionSegment_Volume_triggered();

	virtual void on_actionExit_triggered();

	virtual void on_actionBlank_3D_View_triggered(bool checked);

	virtual void on_actionSet_Camera_triggered();
	virtual void on_actionShow_3D_View_triggered(bool checked);
	virtual void on_actionShow_2D_View_triggered(bool checked);
	virtual void on_actionShow_Region_Table_triggered(bool checked);

	virtual void on_actionChange_Settings_triggered();

	virtual void on_actionData_Loading_triggered();
	virtual void on_actionControls_triggered();
	virtual void on_actionGithub_Repo_triggered();

	virtual void on_actionProject_Website_triggered();

	// Mode events
	virtual void on_actionNavigation();
	virtual void on_actionEdit();
	virtual void on_actionAdd();
	virtual void on_actionMerge();
	virtual void on_actionGrow();
	virtual void on_actionDone();
	virtual void on_actionVisible();

	// Action events
	virtual void on_actionUpdate();
	virtual void on_actionSplit();

	// Tool bar events
	virtual void on_actionOverlay(bool checked);
	virtual void on_actionOutline(bool checked);
	virtual void on_actionRescaleFull();
	virtual void on_actionRescalePartial();
	virtual void on_actionToggleAutoRescale(bool checked);
	virtual void on_actionSmoothNormals(bool checked);
	virtual void on_actionSmoothSurfaces(bool checked);
	virtual void on_actionShowPlane(bool checked);
	virtual void on_actionClearRegionVisibilities();
	virtual void on_actionShowPlaneRegions();
	virtual void on_actionShowNeighborRegions();
	virtual void on_actionFilterRegions(bool checked);

	// Widget events
	virtual void on_sliceUp();
	virtual void on_sliceDown();

	virtual void on_toggleView();
	virtual void on_showBothViews();

	// Region table events
	virtual void on_regionDone(int label, bool done);
	virtual void on_removeRegion(int label);
	virtual void on_highlightRegion(int label);
	virtual void on_selectRegion(int label);
	virtual void on_regionVisible(int label, bool visible);
	virtual void on_regionColor(int label, QColor color);

	// Shortcuts for settings
	virtual void on_overlayDown();
	virtual void on_overlayUp();
	virtual void on_opacityDown();
	virtual void on_opacityUp();
	virtual void on_brushRadiusDown();
	virtual void on_brushRadiusUp();

protected:
	// The visualization container
	VisualizationContainer* visualizationContainer;

	// Region table
	RegionTable* regionTable;

	// Progress bar
	QProgressDialog* progressBar;

	void updateLabels(RegionCollection* regions);

	void updateImage();

	bool eventFilter(QObject* obj, QEvent* event);

	// Default directories
	QString defaultImageDirectoryKey;
	QString defaultSegmentationDirectoryKey;

	QString getDefaultDirectory(QString key);
	void setDefaultDirectory(QString key, QString fileName);

	// Toolbar
	void createModeBar();
	void createToolBar();
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, void (MainWindow::*slot)());
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, bool checked, void (MainWindow::*slot)(bool));
	QLabel* createLabel(const QString& text, int topMargin = 10, int bottomMargin = 5);

	void closeEvent(QCloseEvent* event);
};

#endif
