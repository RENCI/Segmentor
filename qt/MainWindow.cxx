#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QPushButton>
#include <QToolBar>
#include <QShortcut>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QCloseEvent>
#include <QDesktopServices>

#include "VisualizationContainer.h"
#include "Region.h"
#include "RegionCollection.h"
#include "RegionTable.h"
#include "RegionMetadataIO.h"
#include "SliceView.h"
#include "VolumeView.h"
#include "CameraViewDialog.h"
#include "SettingsDialog.h"
#include "SegmentVolumeDialog.h"
#include "SplitRegionDialog.h"
#include "vtkInteractorStyleSlice.h"
#include "FeedbackDialog.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// Constructor
MainWindow::MainWindow() {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	// Progress bar
	progressBar = new QProgressDialog(this);
	progressBar->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	progressBar->setWindowFlag(Qt::WindowCloseButtonHint, false);
	progressBar->setCancelButton(nullptr);
	progressBar->setWindowModality(Qt::WindowModal);
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setMinimumDuration(1000);
	progressBar->reset();

	// Set filenames
	setImageNameLabel("");
	setSegmentationNameLabel("");

	// Default directory key
	defaultDirectoryKey = "default_directory";

	// Create render windows
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Create visualization container
	visualizationContainer = new VisualizationContainer(qvtkWidgetLeft->GetInteractor(), qvtkWidgetRight->GetInteractor(), this);

	// Create mode bar
	createModeBar();

	// Create tool bar
	createToolBar();

	// Create region table
	regionTable = new RegionTable();
	regionTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	regionTableContainer->layout()->addWidget(regionTable);

	QObject::connect(regionTable, &RegionTable::regionDone, this, &MainWindow::on_regionDone);
	QObject::connect(regionTable, &RegionTable::removeRegion, this, &MainWindow::on_removeRegion);
	QObject::connect(regionTable, &RegionTable::highlightRegion, this, &MainWindow::on_highlightRegion);
	QObject::connect(regionTable, &RegionTable::selectRegion, this, &MainWindow::on_selectRegion);
	QObject::connect(regionTable, &RegionTable::regionVisible, this, &MainWindow::on_regionVisible);
	QObject::connect(regionTable, &RegionTable::regionColor, this, &MainWindow::on_regionColor);

	// Settings dialog
	settingsDialog = new SettingsDialog(this, visualizationContainer);
	QObject::connect(settingsDialog, &SettingsDialog::enableDotAnnotationChanged, this, &MainWindow::on_enableDotAnnotationChanged);

	// Feedback dialog
	feedbackDialog = new FeedbackDialog(this, visualizationContainer);
	
	// Slice up and down
	QAction* sliceUpAction = new QAction("+", this);
	sliceUpAction->setShortcut(QKeySequence(Qt::Key_Up));
	sliceUpAction->setToolTip("Move slice up (up arrow)");
	QObject::connect(sliceUpAction, &QAction::triggered, this, &MainWindow::on_sliceUp);

	sliceUpButton->setDefaultAction(sliceUpAction);

	QAction* sliceDownAction = new QAction("-", this);
	sliceDownAction->setShortcut(QKeySequence(Qt::Key_Down));
	sliceDownAction->setToolTip("Move slice down (down arrow)");
	QObject::connect(sliceDownAction, &QAction::triggered, this, &MainWindow::on_sliceDown);

	sliceDownButton->setDefaultAction(sliceDownAction);

	// Window/level
	QObject::connect(this, &MainWindow::windowLevelChanged, settingsDialog, &SettingsDialog::on_windowLevelChanged);

	// Brush radius shortcut
	QShortcut* brushRadiusUp = new QShortcut(QKeySequence(Qt::Key_Right), this);
	QShortcut* brushRadiusDown = new QShortcut(QKeySequence(Qt::Key_Left), this);

	QObject::connect(brushRadiusUp, &QShortcut::activated, this, &MainWindow::on_brushRadiusUp);
	QObject::connect(brushRadiusDown, &QShortcut::activated, this, &MainWindow::on_brushRadiusDown);

	brushRadiusSpinBox->setToolTip("Adjust brush radius (left / right arrow)");

	// Overlay opacity shortcut
	QShortcut* overlayOpacityUp = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this);
	QShortcut* overlayOpacityDown = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this);

	QObject::connect(overlayOpacityUp, &QShortcut::activated, this, &MainWindow::on_overlayOpacityUp);
	QObject::connect(overlayOpacityDown, &QShortcut::activated, this, &MainWindow::on_overlayOpacityDown);
	QObject::connect(this, &MainWindow::overlayOpacityChanged, settingsDialog, &SettingsDialog::on_overlayOpacityChanged);

	// Surface opacity shortcut
	QShortcut* surfaceOpacityUp = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this);
	QShortcut* surfaceOpacityDown = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this);

	QObject::connect(surfaceOpacityUp, &QShortcut::activated, this, &MainWindow::on_surfaceOpacityUp);
	QObject::connect(surfaceOpacityDown, &QShortcut::activated, this, &MainWindow::on_surfaceOpacityDown);
	QObject::connect(this, &MainWindow::surfaceOpacityChanged, settingsDialog, &SettingsDialog::on_surfaceOpacityChanged);

	// 2D/3D toggle
	QShortcut* toggleView = new QShortcut(QKeySequence("t"), this);
	QShortcut* showBothViews = new QShortcut(QKeySequence("r"), this);

	QObject::connect(toggleView, &QShortcut::activated, this, &MainWindow::on_toggleView);
	QObject::connect(showBothViews, &QShortcut::activated, this, &MainWindow::on_showBothViews);

	// Disable most menu items until we have data	
	enableMenus(false);

	qApp->installEventFilter(this);
}

MainWindow::~MainWindow() {
	// Clean up
	delete visualizationContainer;

	qApp->exit();
}

void MainWindow::updateRegions(RegionCollection* regions) {
	regionTable->update(regions);

	updateLabels(regions);
}

void MainWindow::updateRegion(Region* region, RegionCollection* regions) {
	regionTable->update(region);
	feedbackDialog->updateRegion(region);

	updateLabels(regions);
}

void MainWindow::selectRegion(unsigned short label) {
	regionTable->selectRegionLabel(label);
	feedbackDialog->selectRegionLabel(label);
}

void MainWindow::setWindowLevel(double window, double level) {
	emit windowLevelChanged(window, level);
}

void MainWindow::setSlicePosition(double x, double y, double z) {
	QString s = "(" + 
		QString::number(x, 'f', 1) + ", " + 
		QString::number(y, 'f', 1) + ", " + 
		QString::number(z, 'f' , 1) + 
		") ";

	slicePositionLabel->setText(s);
}

void MainWindow::initProgress(QString text) {
	progressBar->setLabelText(text);
}

void MainWindow::updateProgress(double progress) {
	progressBar->setValue(progress * 100);
}

void MainWindow::showMessage(QString message) {
	QMessageBox errorMessage;
	errorMessage.setIcon(QMessageBox::Warning);
	errorMessage.setText(message);
	errorMessage.exec();
}

void MainWindow::on_actionOpen_Image_File_triggered() {
	// Open a file dialog to read the file
	QString file = QFileDialog::getOpenFileName(this,
		"Open Volume",
		getDefaultDirectory(defaultDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;TIFF (*.tif *.tiff);;VTK XML ImageData (*.vti)");

	// Check for file
	if (file == "") {
		return;
	}

	setDefaultDirectory(defaultDirectoryKey, file);

	// Load data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenImageFile(file.toStdString());

	if (errorCode == VisualizationContainer::Success) {
		imagePath = file;
		segmentationPath = "";

		setImageNameLabel(QFileInfo(file).fileName());
		setSegmentationNameLabel("");

		settingsDialog->initializeSettings();

		enableMenus();
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open file.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionOpen_Image_Stack_triggered() {
	// Open a file dialog to read the file
	QString file = QFileDialog::getOpenFileName(this,
		"Open Volume",
		getDefaultDirectory(defaultDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file
	if (file == "") {
		return;
	}

	setDefaultDirectory(defaultDirectoryKey, file);

	// Get all files in directory
	QFileInfo fileInfo(file);
	QDir directory = fileInfo.absoluteDir();
	QFileInfoList fileInfoList = fileInfo.absoluteDir().entryInfoList(QDir::Files, QDir::Name);

	// Check for files
	if (fileInfoList.length() == 0) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");
		errorMessage.setInformativeText("No files present.");
		errorMessage.exec();
		return;
	}

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenImageStack(fileNames);

	if (errorCode == VisualizationContainer::Success) {
		imagePath = directory.absolutePath();
		segmentationPath = "";

		setImageNameLabel(directory.dirName());
		setSegmentationNameLabel("");

		settingsDialog->initializeSettings();

		enableMenus();
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionOpen_Segmentation_File_triggered() {
	// Open a file dialog to read the file
	QString file = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		getDefaultDirectory(defaultDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;TIFF (*.tif *.tiff);;VTK XML ImageData (*.vti)");

	// Check for file
	if (file == "") {
		return;
	}

	setDefaultDirectory(defaultDirectoryKey, file);

	// Load segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenSegmentationFile(file.toStdString());

	if (errorCode == VisualizationContainer::Success) {
		segmentationPath = file;

		setSegmentationNameLabel(QFileInfo(file).fileName());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open file.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		case VisualizationContainer::NoImageData:
			errorMessage.setInformativeText("Load image data first.");
			errorMessage.exec();
			break;

		case VisualizationContainer::VolumeMismatch:
			errorMessage.setInformativeText("Segmentation data volume does not match loaded image data volume.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionOpen_Segmentation_Stack_triggered() {
	// Open a file dialog to read the file
	QString file = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		getDefaultDirectory(defaultDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file
	if (file == "") {
		return;
	}

	setDefaultDirectory(defaultDirectoryKey, file);

	// Get all files in directory
	QFileInfo fileInfo(file);
	QDir directory = fileInfo.absoluteDir();
	QFileInfoList fileInfoList = fileInfo.absoluteDir().entryInfoList(QDir::Files, QDir::Name);

	// Check for files
	if (fileInfoList.length() == 0) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");
		errorMessage.setInformativeText("No files present.");
		errorMessage.exec();
		return;
	}

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenSegmentationStack(fileNames);

	if (errorCode == VisualizationContainer::Success) {
		segmentationPath = directory.absolutePath();

		setSegmentationNameLabel(directory.dirName());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not open files.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		case VisualizationContainer::NoImageData:
			errorMessage.setInformativeText("Load image data first.");
			errorMessage.exec();
			break;

		case VisualizationContainer::VolumeMismatch:
			errorMessage.setInformativeText("Segmentation data volume does not match loaded image data volume.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionSave_Image_Data_As_triggered() {
	// Open a file dialog to save the file
	QString file = QFileDialog::getSaveFileName(this,
		"Save Image Data",
		getDefaultDirectory(defaultDirectoryKey),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file
	if (file == "") {
		return;
	}

	// Check against segmentation file name
	if (file == segmentationPath) {
		if (QMessageBox::warning(this, "Warning", 
			"About to save image data as current segmentation file.\n\nThis is probably a bad idea.\n\n Do you wish to proceed?", 
			QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
			return;
		}
	}	

	setDefaultDirectory(defaultDirectoryKey, file);

	// Get just the file name
	QString fileName = QFileInfo(file).fileName();

	// Save image data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveImageData(file.toStdString());

	if (errorCode == VisualizationContainer::Success) {
		imagePath = file;

		setImageNameLabel(fileName);
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not save data.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionSave_Segmentation_Data_triggered() {
	// Save segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveSegmentationData();

	if (errorCode == VisualizationContainer::NoFileName) {
		on_actionSave_Segmentation_Data_As_triggered();
	}
	else if (errorCode != VisualizationContainer::Success) {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not save data.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionSave_Segmentation_Data_As_triggered() {
	// Open a file dialog to save the file
	QString file = QFileDialog::getSaveFileName(this,
		"Save Segmentation Data",
		getDefaultDirectory(defaultDirectoryKey),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file
	if (file == "") {
		return;
	}
	
	// Check against image file name
	if (file == imagePath) {
		if (QMessageBox::warning(this, "Warning",
			"About to save segmentation data as current image file.\n\nThis is probably a bad idea.\n\n Do you wish to proceed?",
			QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
			return;
		}
	}

	setDefaultDirectory(defaultDirectoryKey, file);

	// Get just the file name
	QString fileName = QFileInfo(file).fileName();

	// Save segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveSegmentationData(file.toStdString());

	if (errorCode == VisualizationContainer::Success) {
		segmentationPath = file;

		setSegmentationNameLabel(fileName);
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setIcon(QMessageBox::Warning);
		errorMessage.setText("Could not save data.");

		switch (errorCode) {
		case VisualizationContainer::WrongFileType:
			errorMessage.setInformativeText("Wrong file type.");
			errorMessage.exec();
			break;

		default:
			errorMessage.setInformativeText("Unknown error.");
			errorMessage.exec();
		}
	}
}

void MainWindow::on_actionUndo_triggered() {
	visualizationContainer->Undo();
}

void MainWindow::on_actionRedo_triggered() {
	visualizationContainer->Redo();
}

void MainWindow::on_actionSegment_Volume_triggered() {
	visualizationContainer->PushTempHistory();

	SegmentVolumeDialog dialog(this, visualizationContainer);

	if (dialog.exec()) {
		setSegmentationNameLabel("Generated");
	}
	else {
		visualizationContainer->PopTempHistory();
	}
}

void MainWindow::on_actionExit_triggered() {
	close();
}

void MainWindow::on_actionBlank_3D_View_triggered(bool checked) {
	visualizationContainer->GetVolumeView()->Enable(!checked);
}

void MainWindow::on_actionSet_Camera_triggered() {
	double cameraPos[3];
	double slicePos[3];

	vtkRenderer* renderer = visualizationContainer->GetSliceView()->GetRenderer();
	vtkCamera* camera = renderer->GetActiveCamera();

	camera->GetPosition(cameraPos);
	camera->GetFocalPoint(slicePos);

	CameraViewDialog dialog(this, renderer);
	
	if (!dialog.exec()) {
		camera->SetPosition(cameraPos);
		camera->SetFocalPoint(slicePos);

		visualizationContainer->Render();
	}
}

void MainWindow::on_actionShow_3D_View_triggered(bool checked) {
	qvtkWidgetLeft->setVisible(checked);
	visualizationContainer->GetVolumeView()->GetRenderer()->SetDraw(checked);
	
	actionShow_2D_View->setEnabled(checked);
}

void MainWindow::on_actionShow_2D_View_triggered(bool checked) {
	qvtkWidgetRight->setVisible(checked);
	visualizationContainer->GetSliceView()->GetRenderer()->SetDraw(checked);

	actionShow_3D_View->setEnabled(checked);
}

void MainWindow::on_actionShow_Region_Table_triggered(bool checked) {
	regionTableContainer->setVisible(!regionTableContainer->isVisible());
}

void MainWindow::on_actionChange_Settings_triggered() {
	settingsDialog->initializeSettings();
	settingsDialog->show();
	settingsDialog->raise();
	settingsDialog->activateWindow();
}

void MainWindow::on_actionShow_Feedback_triggered() {
	feedbackDialog->updateRegions();
	feedbackDialog->show();
	feedbackDialog->raise();
	feedbackDialog->activateWindow();
}

void MainWindow::on_actionData_Loading_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/RENCI/Segmentor/wiki/Data-Loading-and-Saving"));
}

void MainWindow::on_actionControls_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/RENCI/Segmentor/wiki/Controls"));
}
void MainWindow::on_actionGithub_Repo_triggered() {
	QDesktopServices::openUrl(QUrl("https://github.com/RENCI/Segmentor"));
}

void MainWindow::on_actionProject_Website_triggered() {
	QDesktopServices::openUrl(QUrl("https://www.nucleininja.org/"));
}

void MainWindow::on_actionNavigation() {
	visualizationContainer->SetInteractionMode(NavigationMode);

	navEditMode = NavigationMode;
}

void MainWindow::on_actionEdit() {
	visualizationContainer->SetInteractionMode(EditMode);

	navEditMode = EditMode;
}

void MainWindow::on_actionAdd() {
	visualizationContainer->SetInteractionMode(AddMode);
}

void MainWindow::on_actionMerge() {
	visualizationContainer->SetInteractionMode(MergeMode);
}

void MainWindow::on_actionGrow() {
	visualizationContainer->SetInteractionMode(GrowMode);
}

void MainWindow::on_actionVisible() {
	visualizationContainer->SetInteractionMode(VisibleMode);
}

void MainWindow::on_actionDot() {
	visualizationContainer->SetInteractionMode(DotMode);
}

void MainWindow::on_actionUpdate() {
	visualizationContainer->RelabelCurrentRegion();
}

void MainWindow::on_actionClean() {
	visualizationContainer->CleanCurrentRegion();
}

void MainWindow::on_actionSplit() {
	// Split in two
	visualizationContainer->SplitCurrentRegion(2);
}

void MainWindow::on_actionSplitMultiple() {
	SplitRegionDialog dialog(this, visualizationContainer);

	dialog.exec();
}

void MainWindow::on_actionFill() {
	visualizationContainer->FillCurrentRegionSlice();
}

void MainWindow::on_actionDone() {
	visualizationContainer->ToggleCurrentRegionDone();
}

void MainWindow::on_actionOverlay(bool checked) {
	visualizationContainer->GetSliceView()->ShowLabelSlice(checked);
}

void MainWindow::on_actionOutline(bool checked) {
	visualizationContainer->GetSliceView()->ShowRegionOutlines(checked);
}

void MainWindow::on_actionRescaleFull() {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	sliceView->RescaleFull();

//	setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());
}

void MainWindow::on_actionRescalePartial() {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	sliceView->RescalePartial();

//	setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());
}

void MainWindow::on_actionToggleAutoRescale(bool checked) {
	visualizationContainer->GetSliceView()->SetAutoRescale(checked);
}

void MainWindow::on_actionSmoothNormals(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothShading(checked);
}

void MainWindow::on_actionSmoothSurfaces(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothSurfaces(checked);
}

void MainWindow::on_actionVolumeRendering(bool checked) {
	visualizationContainer->GetVolumeView()->SetVolumeRendering(checked);
}

void MainWindow::on_actionShowPlane(bool checked) {
	visualizationContainer->GetVolumeView()->SetShowPlane(checked);
}

void MainWindow::on_actionClearRegionVisibilities() {
	visualizationContainer->ClearRegionVisibilities();
}

void MainWindow::on_actionShowPlaneRegions() {
	visualizationContainer->ShowPlaneRegions();
}

void MainWindow::on_actionShowNeighborRegions() {
	visualizationContainer->ShowNeighborRegions();
}

void MainWindow::on_actionFilterRegions(bool checked) {
	visualizationContainer->SetFilterRegions(checked);
}

void MainWindow::on_actionViewX() {
	visualizationContainer->GetSliceView()->SetOrientationX();
}

void MainWindow::on_actionViewY() {
	visualizationContainer->GetSliceView()->SetOrientationY();
}

void MainWindow::on_actionViewZ() {
	visualizationContainer->GetSliceView()->SetOrientationZ();
}

void MainWindow::on_actionResetView() {
	visualizationContainer->GetSliceView()->GetRenderer()->ResetCamera();
	visualizationContainer->Render();
}

void MainWindow::on_sliceUp() {
	visualizationContainer->SliceUp();
}

void MainWindow::on_sliceDown() {
	visualizationContainer->SliceDown();
}

void MainWindow::on_brushRadiusSpinBox_valueChanged(int value) {
	visualizationContainer->SetBrushRadius(value);
}

void MainWindow::on_enableDotAnnotationChanged(bool enable) {
	QList<QAction*> actions = modeBarWidget->findChild<QToolBar*>()->actions();

	for (int i = 0; i < actions.length(); i++) {
		QAction* action = actions[i];
		if (action->objectName() == "dotModeAction") {
			action->setEnabled(enable);
			break;
		}
	}
}

void MainWindow::on_toggleView() {
	if (qvtkWidgetLeft->isVisible()) {
		qvtkWidgetLeft->setVisible(false);
		qvtkWidgetRight->setVisible(true);

		visualizationContainer->GetVolumeView()->GetRenderer()->SetDraw(false);
		visualizationContainer->GetSliceView()->GetRenderer()->SetDraw(true);

		actionShow_3D_View->setEnabled(true);
		actionShow_2D_View->setEnabled(false);
	}
	else {
		qvtkWidgetLeft->setVisible(true);
		qvtkWidgetRight->setVisible(false);

		visualizationContainer->GetVolumeView()->GetRenderer()->SetDraw(true);
		visualizationContainer->GetSliceView()->GetRenderer()->SetDraw(false);

		actionShow_3D_View->setEnabled(false);
		actionShow_2D_View->setEnabled(true);
	}
}

void MainWindow::on_showBothViews() {
	qvtkWidgetLeft->setVisible(true);
	qvtkWidgetRight->setVisible(true);

	visualizationContainer->GetVolumeView()->GetRenderer()->SetDraw(true);
	visualizationContainer->GetSliceView()->GetRenderer()->SetDraw(true);

	actionShow_3D_View->setEnabled(true);
	actionShow_2D_View->setEnabled(true);
}

void MainWindow::on_regionDone(int label, bool done) {
	Region* region = visualizationContainer->SetRegionDone((unsigned short)label, done);

	if (done && !region->GetDone()) {
		regionTable->update(region);
	}
}

void MainWindow::on_removeRegion(int label) {
	visualizationContainer->RemoveRegion((unsigned short)label);
}

void MainWindow::on_highlightRegion(int label) {
	visualizationContainer->HighlightRegion((unsigned short)label);
}

void MainWindow::on_selectRegion(int label) {
	visualizationContainer->SelectRegion((unsigned short)label);
}

void MainWindow::on_regionVisible(int label, bool visible) {
	visualizationContainer->SetRegionVisibility((unsigned short)label, visible);
}

void MainWindow::on_regionColor(int label, QColor color) {
	visualizationContainer->SetRegionColor((unsigned short)label, color.redF(), color.greenF(), color.blueF());
}

void MainWindow::on_overlayOpacityDown() {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	double value = qMax(sliceView->GetOverlayOpacity() - 0.1, 0.0);

	sliceView->SetOverlayOpacity(value);

	emit overlayOpacityChanged(value);
}

void MainWindow::on_overlayOpacityUp() {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	double value = qMin(sliceView->GetOverlayOpacity() + 0.1, 1.0);

	sliceView->SetOverlayOpacity(value);

	emit overlayOpacityChanged(value);
}

void MainWindow::on_surfaceOpacityDown() {
	double value = qMax(visualizationContainer->GetVolumeView()->GetVisibleOpacity() - 0.1, 0.0);

	visualizationContainer->SetVisibleOpacity(value);

	emit surfaceOpacityChanged(value);
}

void MainWindow::on_surfaceOpacityUp() {
	double value = qMin(visualizationContainer->GetVolumeView()->GetVisibleOpacity() + 0.1, 1.0);

	visualizationContainer->SetVisibleOpacity(value);

	emit surfaceOpacityChanged(value);
}

void MainWindow::on_brushRadiusDown() {
	int value = qMax(visualizationContainer->GetBrushRadius() - 1, brushRadiusSpinBox->minimum());

	brushRadiusSpinBox->setValue(value);
}

void MainWindow::on_brushRadiusUp() {
	int value = qMin(visualizationContainer->GetBrushRadius() + 1, brushRadiusSpinBox->maximum());

	brushRadiusSpinBox->setValue(value);
}

void MainWindow::updateLabels(RegionCollection* regions) {
	int refiningCount = 0, doneCount = 0;
	int minSize = 0, maxSize = 0, total = 0;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);

		int size = region->GetNumVoxels();

		if (it == regions->Begin()) {
			minSize = maxSize = size;
		}
		else {
			minSize = std::min(minSize, size);
			maxSize = std::max(maxSize, size);
		}

		total += size;

		if (region->GetModified() && !region->GetDone()) refiningCount++;
		if (region->GetDone()) doneCount++;
	}

	int n = regions->Size();

	total_Label->setText("Total: " + QString::number(n));
	refining_Label->setText("Refining: " + QString::number(refiningCount));
	done_Label->setText("Done: " + QString::number(doneCount));

	min_Label->setText("Minimum: " + QString::number(minSize));
	max_Label->setText("Maximum: " + QString::number(maxSize));
	median_Label->setText("Median: " + QString::number(n > 0 ? (double)total / n : 0, 'f', 1));
}

/*
void MainWindow::updateImage() {
	const double* range = visualizationContainer->GetDataRange();
	
	double step = (range[1] - range[0]) / 25;

	windowSpinBox->setSingleStep(step);
	levelSpinBox->setSingleStep(step);
}
*/

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
	if (event->type() == QEvent::Enter) {
		if (obj == qvtkWidgetLeft) {
			//std::cout << "LEFT" << std::endl;

			// XXX: How to grab focus so keypresses work without clicking first?
		}
		else if (obj == qvtkWidgetRight) {
			//std::cout << "RIGHT" << std::endl;

			// XXX: How to grab focus so keypresses work without clicking first?
		}
	}

	return QObject::eventFilter(obj, event);
}

QString MainWindow::getDefaultDirectory(QString key) {
	QSettings settings;

	return settings.value(key).toString();
}

void MainWindow::setDefaultDirectory(QString key, QString fileName) {
	QFileInfo fileInfo(fileName);

	QSettings settings;
	settings.setValue(key, fileInfo.absoluteDir().absolutePath());
}

void MainWindow::setImageNameLabel(QString name) {
	imageNameLabel->setText("Image: " + name);
}

void MainWindow::setSegmentationNameLabel(QString name) {
	segmentationNameLabel->setText("Segmentation: " + name);
}

void MainWindow::createModeBar() {
	// Create tool bar
	QToolBar* toolBar = new QToolBar();
	toolBar->setFloatable(true);
	toolBar->setMovable(true);
	toolBar->setOrientation(Qt::Horizontal);
	toolBar->setIconSize(QSize(20, 20));

	InteractionMode currentMode = visualizationContainer->GetInteractionMode();

	// Save navigation / edit for switching back to them from other modes
	navEditMode = currentMode == EditMode ? EditMode : NavigationMode;

	// Interaction toggle
	QActionGroup* interactionModeGroup = new QActionGroup(this);
	interactionModeGroup->setExclusive(true);

	QAction* actionNavigation = new QAction(QIcon(":/icons/icon_navigation.png"), "Navigation mode (space bar)", interactionModeGroup);
	actionNavigation->setCheckable(true);
	actionNavigation->setChecked(currentMode == NavigationMode);

	QAction* actionEdit = new QAction(QIcon(":/icons/icon_edit.png"), "Edit mode (space bar)", interactionModeGroup);
	actionEdit->setCheckable(true);
	actionEdit->setChecked(currentMode == EditMode);

	QObject::connect(actionNavigation, &QAction::triggered, this, &MainWindow::on_actionNavigation);
	QObject::connect(actionEdit, &QAction::triggered, this, &MainWindow::on_actionEdit);
	
	QObject::connect(new QShortcut(QKeySequence(Qt::Key_Space), this), &QShortcut::activated, [actionNavigation, actionEdit, this]() {
		if (actionNavigation->isChecked()) {
			actionEdit->toggle();
			emit(actionEdit->triggered(true));
		}
		else if (actionEdit->isChecked()) {
			actionNavigation->toggle();
			emit(actionNavigation->triggered(true));
		}
		else if (navEditMode == NavigationMode) {
			actionNavigation->toggle();
			emit(actionNavigation->triggered(true));
		}
		else if (navEditMode == EditMode) {
			actionEdit->toggle();
			emit(actionEdit->triggered(true));
		}
		else {
			actionNavigation->toggle();
			emit(actionNavigation->triggered(true));
		}
	});

	// Add widgets to tool bar
	toolBar->addWidget(createLabel("Mode", 0, 0, 0, 5));
	toolBar->addAction(actionNavigation);
	toolBar->addAction(actionEdit);
	toolBar->addAction(createActionIcon(":/icons/icon_add.png", "Add region (a)", "a", interactionModeGroup, currentMode == AddMode, &MainWindow::on_actionAdd));
	toolBar->addAction(createActionIcon(":/icons/icon_merge.png", "Merge with current region (m)", "m", interactionModeGroup, currentMode == MergeMode, &MainWindow::on_actionMerge));
	toolBar->addAction(createActionIcon(":/icons/icon_grow.png", "Grow / shrink region (g)", "g", interactionModeGroup, currentMode == GrowMode, &MainWindow::on_actionGrow));
	toolBar->addAction(createActionIcon(":/icons/icon_visible.png", "Toggle region visibility (v)", "v", interactionModeGroup, currentMode == VisibleMode, &MainWindow::on_actionVisible));

	QAction* dotAction = createActionIcon(":/icons/icon_dot.png", "Dot annotation mode (Ctrl + d)", QKeySequence(Qt::CTRL + Qt::Key_D), interactionModeGroup, currentMode == DotMode, &MainWindow::on_actionDot);
	dotAction->setObjectName("dotModeAction");
	dotAction->setEnabled(false);
	toolBar->addAction(dotAction);
	
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("Actions", 0, 0, 5, 5));
	toolBar->addAction(createActionIcon(":/icons/icon_fill.png", "Fill current region slice (l)", "l", &MainWindow::on_actionFill));
	toolBar->addAction(createActionIcon(":/icons/icon_update.png", "Update current region (u)", "u", &MainWindow::on_actionUpdate));
	toolBar->addAction(createActionIcon(":/icons/icon_clean.png", "Clean current region (0)", "0", &MainWindow::on_actionClean));
	toolBar->addAction(createActionIcon(":/icons/icon_split.png", "Split current region (/)", "/", &MainWindow::on_actionSplit));
	toolBar->addAction(createActionIcon(":/icons/icon_split_multiple.png", "Split current region into multiple regions (Ctrl + /)", QKeySequence(Qt::CTRL + Qt::Key_Slash), &MainWindow::on_actionSplitMultiple));
	toolBar->addAction(createActionIcon(":/icons/icon_done.png", "Toggle current region done (d)", "d", &MainWindow::on_actionDone));

	modeBarWidget->layout()->addWidget(toolBar);
}

void MainWindow::createToolBar() {
	// Create tool bar
	QToolBar* toolBar = new QToolBar();
	toolBar->setFloatable(true);
	toolBar->setMovable(true);
	toolBar->setOrientation(Qt::Vertical);
	toolBar->setIconSize(QSize(20, 20));

	// Add widgets to tool bar
	toolBar->addWidget(createLabel("2D"));
	toolBar->addAction(createActionIcon(":/icons/icon_overlay.png", "Show overlay (q)", "q", visualizationContainer->GetSliceView()->GetShowLabelSlice(), &MainWindow::on_actionOverlay));
	toolBar->addAction(createActionIcon(":/icons/icon_outline.png", "Show outlines (e)", "e", visualizationContainer->GetSliceView()->GetShowRegionOutlines(), &MainWindow::on_actionOutline));
	toolBar->addAction(createActionIcon(":/icons/icon_rescale_full.png", "Rescale full (=)", "=", &MainWindow::on_actionRescaleFull));
	toolBar->addAction(createActionIcon(":/icons/icon_rescale_partial.png", "Rescale partial (-)", "-", &MainWindow::on_actionRescalePartial));
	toolBar->addAction(createActionIcon(":/icons/icon_rescale_auto.png", "Rescale auto ([)", "[", visualizationContainer->GetSliceView()->GetAutoRescale(), &MainWindow::on_actionToggleAutoRescale));
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("3D"));
	toolBar->addAction(createActionIcon(":/icons/icon_smooth_normals.png", "Smooth normals (n)", "n", visualizationContainer->GetVolumeView()->GetSmoothShading(), &MainWindow::on_actionSmoothNormals));
	toolBar->addAction(createActionIcon(":/icons/icon_smooth_surface.png", "Smooth surfaces (s)", "s", visualizationContainer->GetVolumeView()->GetSmoothSurfaces(), &MainWindow::on_actionSmoothSurfaces));
	toolBar->addAction(createActionIcon(":/icons/icon_volume_render.png", "Volume rendering (])", "]", visualizationContainer->GetVolumeView()->GetVolumeRendering(), &MainWindow::on_actionVolumeRendering));
	toolBar->addAction(createActionIcon(":/icons/icon_plane.png", "Show plane (o)", "o", visualizationContainer->GetVolumeView()->GetShowPlane(), &MainWindow::on_actionShowPlane));
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("Filter"));
	toolBar->addAction(createActionIcon(":/icons/icon_filter_regions.png", "Filter regions (b)", "b", visualizationContainer->GetFilterRegions(), &MainWindow::on_actionFilterRegions));
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("Visibility"));
	toolBar->addAction(createActionIcon(":/icons/icon_clear_visibility.png", "Clear visibility (c)", "c", &MainWindow::on_actionClearRegionVisibilities));
	toolBar->addAction(createActionIcon(":/icons/icon_show_plane_regions.png", "Show plane regions (p)", "p", &MainWindow::on_actionShowPlaneRegions));
	toolBar->addAction(createActionIcon(":/icons/icon_show_neighbor_regions.png", "Show neighbors (k)", "k", &MainWindow::on_actionShowNeighborRegions));
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("View"));
	toolBar->addAction(createAction("X", "x", &MainWindow::on_actionViewX));
	toolBar->addAction(createAction("Y", "y", &MainWindow::on_actionViewY));
	toolBar->addAction(createAction("Z", "z", &MainWindow::on_actionViewZ));
	toolBar->addAction(createActionIcon(":/icons/icon_reset_view.png", "Reset view (i)", "i", &MainWindow::on_actionResetView));

	toolBarWidget->layout()->addWidget(toolBar);
}

QAction* MainWindow::createAction(const QString& text, const QString& shortcut, void (MainWindow::*slot)()) {
	QAction* action = new QAction(text, this);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(false);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QAction* MainWindow::createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, void (MainWindow::*slot)()) {
	QAction* action = new QAction(QIcon(fileName), text, this);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(false);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QAction* MainWindow::createActionIcon(const QString& fileName, const QString& text, const QKeySequence& shortcut, void (MainWindow::*slot)()) {
	QAction* action = new QAction(QIcon(fileName), text, this);
	action->setShortcut(shortcut);
	action->setCheckable(false);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QAction* MainWindow::createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, bool checked, void (MainWindow::*slot)(bool)) {
	QAction* action = new QAction(QIcon(fileName), text, this);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(true);
	action->setChecked(checked);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QAction* MainWindow::createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, QActionGroup* group, bool checked, void (MainWindow::*slot)()) {
	QAction* action = new QAction(QIcon(fileName), text, group);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(true);
	action->setChecked(checked);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QAction* MainWindow::createActionIcon(const QString& fileName, const QString& text, const QKeySequence& shortcut, QActionGroup* group, bool checked, void (MainWindow::*slot)()) {
	QAction* action = new QAction(QIcon(fileName), text, group);
	action->setShortcut(QKeySequence(shortcut));
	action->setCheckable(true);
	action->setChecked(checked);

	QObject::connect(action, &QAction::triggered, this, slot);

	return action;
}

QLabel* MainWindow::createLabel(const QString& text, int topMargin, int bottomMargin, int leftMargin, int rightMargin) {
	QString style = QStringLiteral("color:#999;margin-top:%1px;margin-bottom:%2px;margin-left:%3px;margin-right:%4px")
		.arg(topMargin).arg(bottomMargin).arg(leftMargin).arg(rightMargin);

	QLabel* label = new QLabel(text);
	label->setAlignment(Qt::AlignCenter);
	label->setStyleSheet(style);

	return label;
}

void MainWindow::enableMenus(bool enable) {
	menuEdit->setEnabled(enable);
	menuAnalyze->setEnabled(enable);
	menuView->setEnabled(enable);
	menuSettings->setEnabled(enable);
	menuTraining->setEnabled(enable);

	modeBarWidget->setEnabled(enable);
	toolBarWidget->setEnabled(enable);

	sliceDownButton->setEnabled(enable);
	sliceUpButton->setEnabled(enable);
}

void MainWindow::closeEvent(QCloseEvent* event) {
	if (visualizationContainer->NeedToSave()) {
		QMessageBox message;
		message.setIcon(QMessageBox::Warning);
		message.setText("Unsaved changes.");
		message.setInformativeText("Do you want to save your changes?");
		message.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		message.setDefaultButton(QMessageBox::Save);
		int ret = message.exec();

		switch (ret) {
		case QMessageBox::Save:
			on_actionSave_Segmentation_Data_triggered();
			event->accept();
			break;

		case QMessageBox::Discard:
			event->accept();
			break;

		case QMessageBox::Cancel:
			event->ignore();
			break;

		default:
			// Should never be reached
			event->ignore();
			break;
		}
	}
	else {
		event->accept();
	}
};