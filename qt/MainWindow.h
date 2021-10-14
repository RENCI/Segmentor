#ifndef MainWindow_H
#define MainWindow_H

#include "ui_MainWindow.h"

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QProgressDialog>
#include <QSettings>

#include <string>

#include "InteractionEnums.h"

class VisualizationContainer;
class Region;
class RegionCollection;
class RegionTable;
class SettingsDialog;
class FeedbackDialog;

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

	void initProgress(QString text);
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
	virtual void on_actionApply_Dot_Annotation_triggered();

	virtual void on_actionExit_triggered();

	virtual void on_actionBlank_3D_View_triggered(bool checked);

	virtual void on_actionSet_Camera_triggered();
	virtual void on_actionShow_3D_View_triggered(bool checked);
	virtual void on_actionShow_2D_View_triggered(bool checked);
	virtual void on_actionShow_Region_Table_triggered(bool checked);

	virtual void on_actionChange_Settings_triggered();

	virtual void on_actionShow_Feedback_triggered();

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
	virtual void on_actionDot();

	// Action events
	virtual void on_actionUpdate();
	virtual void on_actionClean();
	virtual void on_actionSplit();
	virtual void on_actionSplitMultiple();
	virtual void on_actionFill();

	// Tool bar events
	virtual void on_actionOverlay(bool checked);
	virtual void on_actionOutline(bool checked);
	virtual void on_actionRescaleFull();
	virtual void on_actionRescalePartial();
	virtual void on_actionToggleAutoRescale(bool checked);
	virtual void on_actionSmoothNormals(bool checked);
	virtual void on_actionSmoothSurfaces(bool checked);
	virtual void on_actionVolumeRendering(bool checked);
	virtual void on_actionShowPlane(bool checked);
	virtual void on_actionClearRegionVisibilities();
	virtual void on_actionShowPlaneRegions();
	virtual void on_actionShowNeighborRegions();
	virtual void on_actionFilterRegions(bool checked);
	virtual void on_actionViewX();
	virtual void on_actionViewY();
	virtual void on_actionViewZ();
	virtual void on_actionResetView();

	// Widget events
	virtual void on_sliceUp();
	virtual void on_sliceDown();

	virtual void on_brushRadiusSpinBox_valueChanged(int value);

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
	virtual void on_overlayOpacityDown();
	virtual void on_overlayOpacityUp();
	virtual void on_surfaceOpacityDown();
	virtual void on_surfaceOpacityUp();
	virtual void on_brushRadiusDown();
	virtual void on_brushRadiusUp();

	virtual void on_enableDotAnnotationChanged(bool enable);

signals:

	void windowLevelChanged(double window, double value);
	void overlayOpacityChanged(double value);
	void surfaceOpacityChanged(double value);

protected:
	// The visualization container
	VisualizationContainer* visualizationContainer;

	// Region table
	RegionTable* regionTable;

	// Progress bar
	QProgressDialog* progressBar;

	void updateLabels(RegionCollection* regions);

	bool eventFilter(QObject* obj, QEvent* event);

	// Default directory
	QString defaultDirectoryKey;

	QString getDefaultDirectory(QString key);
	void setDefaultDirectory(QString key, QString fileName);

	// Filename labels
	void setImageNameLabel(QString name);
	void setSegmentationNameLabel(QString name);

	// File paths
	QString imagePath;
	QString segmentationPath;

	// Toolbar
	void createModeBar();
	void createToolBar();
	QAction* createAction(const QString& text, const QString& shortcut, void (MainWindow::*slot)());
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, void (MainWindow::*slot)());
	QAction* createActionIcon(const QString& fileName, const QString& text, const QKeySequence& shortcut, void (MainWindow::*slot)());
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, bool checked, void (MainWindow::*slot)(bool));
	QAction* createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, QActionGroup* group, bool checked, void (MainWindow::*slot)());
	QAction* createActionIcon(const QString& fileName, const QString& text, const QKeySequence& shortcut, QActionGroup* group, bool checked, void (MainWindow::*slot)());
	QLabel* createLabel(const QString& text, int topMargin = 10, int bottomMargin = 5, int leftMargin = 0, int rightMargin = 0);

	InteractionMode navEditMode;

	// Dialogs
	SettingsDialog* settingsDialog;
	FeedbackDialog* feedbackDialog;

	// Disable menus
	void enableMenus(bool enable = true);

	void closeEvent(QCloseEvent* event);
};

#endif
