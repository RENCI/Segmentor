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
#include "InteractionEnums.h"
#include "SliceView.h"
#include "VolumeView.h"
#include "CameraViewDialog.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

// Constructor
MainWindow::MainWindow() {
	progressBar = new QProgressDialog("Processing segmentation data", "", 0, 100, this);
	progressBar->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	progressBar->setWindowFlag(Qt::WindowCloseButtonHint, false);
	progressBar->setCancelButton(nullptr);
	progressBar->setWindowModality(Qt::WindowModal);
	progressBar->reset();

	// Create the GUI from the Qt Designer file
	setupUi(this);

	// Default directory keys
	defaultImageDirectoryKey = "default_image_directory";
	defaultSegmentationDirectoryKey = "default_segmentation_directory";

	// Create render windows
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Create visualization container
	visualizationContainer = new VisualizationContainer(qvtkWidgetLeft->GetInteractor(), qvtkWidgetRight->GetInteractor(), this);

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

	// Voxel size
	QObject::connect(xSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::on_voxelSizeSpinBox);
	QObject::connect(ySizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::on_voxelSizeSpinBox);
	QObject::connect(zSizeSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::on_voxelSizeSpinBox);

	// Window/level range
	windowSpinBox->setMinimum(0);
	windowSpinBox->setMaximum(9999);
	windowSpinBox->setSingleStep(100);
	windowSpinBox->setDecimals(1);

	levelSpinBox->setMinimum(-9999);
	levelSpinBox->setMaximum(9999);
	levelSpinBox->setSingleStep(100);
	levelSpinBox->setDecimals(1);

	// Overlay opacity
	QShortcut* overlayUp = new QShortcut(QKeySequence(Qt::Key_Right), this);
	QShortcut* overlayDown = new QShortcut(QKeySequence(Qt::Key_Left), this);

	QObject::connect(overlayUp, &QShortcut::activated, this, &MainWindow::on_overlayUp);
	QObject::connect(overlayDown, &QShortcut::activated, this, &MainWindow::on_overlayDown);

	overlaySpinBox->valueChanged(overlaySpinBox->value());

	// Opacity
	QShortcut* opacityUp = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Right), this);
	QShortcut* opacityDown = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Left), this);

	QObject::connect(opacityUp, &QShortcut::activated, this, &MainWindow::on_opacityUp);
	QObject::connect(opacityDown, &QShortcut::activated, this, &MainWindow::on_opacityDown);

	opacitySpinBox->valueChanged(opacitySpinBox->value());

	// Brush radius
	QShortcut* brushRadiusUp = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Right), this);
	QShortcut* brushRadiusDown = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Left), this);

	QObject::connect(brushRadiusUp, &QShortcut::activated, this, &MainWindow::on_brushRadiusUp);
	QObject::connect(brushRadiusDown, &QShortcut::activated, this, &MainWindow::on_brushRadiusDown);

	brushRadiusSpinBox->valueChanged(brushRadiusSpinBox->value());

	// 2D/3D toggle
	QShortcut* toggleView = new QShortcut(QKeySequence("t"), this);
	QShortcut* showBothViews = new QShortcut(QKeySequence("r"), this);

	QObject::connect(toggleView, &QShortcut::activated, this, &MainWindow::on_toggleView);
	QObject::connect(showBothViews, &QShortcut::activated, this, &MainWindow::on_showBothViews);

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

	updateLabels(regions);
}

void MainWindow::selectRegion(unsigned short label) {
	regionTable->selectRegionLabel(label);
}

void MainWindow::setWindowLevel(double window, double level) {
	windowSpinBox->setValue(window);
	levelSpinBox->setValue(level);
}

void MainWindow::setSlicePosition(double x, double y, double z) {
	QString s = "Slice Position: (" + 
		QString::number(x, 'f', 1) + ", " + 
		QString::number(y, 'f', 1) + ", " + 
		QString::number(z, 'f' , 1) + ")";

	slicePositionLabel->setText(s);
}

void MainWindow::setVoxelSize(double x, double y, double z) {
	xSizeSpinBox->setValue(x);
	ySizeSpinBox->setValue(y);
	zSizeSpinBox->setValue(z);
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
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		getDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;TIFF (*.tif *.tiff);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	setDefaultDirectory(defaultImageDirectoryKey, fileName);

	// Load data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenImageFile(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
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
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		getDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	setDefaultDirectory(defaultImageDirectoryKey, fileName);

	// Get all files in directory
	QFileInfo fileInfo(fileName);
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

	if (errorCode != VisualizationContainer::Success) {
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
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		getDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;TIFF (*.tif *.tiff);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	setDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

	// Progress bar
	

	// Load segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenSegmentationFile(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
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
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		getDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	setDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

	// Get all files in directory
	QFileInfo fileInfo(fileName);
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

	if (errorCode != VisualizationContainer::Success) {
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
	QString fileName = QFileDialog::getSaveFileName(this,
		"Save Image Data",
		getDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	setDefaultDirectory(defaultImageDirectoryKey, fileName);

	// Save image data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveImageData(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
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
	QString fileName = QFileDialog::getSaveFileName(this,
		"Save Segmentation Data",
		getDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	setDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

	// Save segmentation data
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->SaveSegmentationData(fileName.toStdString());

	if (errorCode != VisualizationContainer::Success) {
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
	visualizationContainer->SegmentVolume();
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
}

void MainWindow::on_actionEdit() {
	visualizationContainer->SetInteractionMode(EditMode);
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

	setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());
}

void MainWindow::on_actionRescalePartial() {
	SliceView* sliceView = visualizationContainer->GetSliceView();

	sliceView->RescalePartial();

	setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());
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

void MainWindow::on_sliceUp() {
	visualizationContainer->SliceUp();
}

void MainWindow::on_sliceDown() {
	visualizationContainer->SliceDown();
}

void MainWindow::on_windowSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetWindow(value);
}

void MainWindow::on_levelSpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetLevel(value);
}

void MainWindow::on_overlaySpinBox_valueChanged(double value) {
	visualizationContainer->GetSliceView()->SetOverlayOpacity(value);
}

void MainWindow::on_overlayUp() {
	overlaySpinBox->stepUp();
}

void MainWindow::on_overlayDown() {
	overlaySpinBox->stepDown();
}

void MainWindow::on_opacitySpinBox_valueChanged(double value) {
	visualizationContainer->SetVisibleOpacity(value);
}

void MainWindow::on_opacityUp() {
	opacitySpinBox->stepUp();
}

void MainWindow::on_opacityDown() {
	opacitySpinBox->stepDown();
}

void MainWindow::on_voxelSizeSpinBox() {
	visualizationContainer->SetVoxelSize(
		xSizeSpinBox->value(),
		ySizeSpinBox->value(),
		zSizeSpinBox->value()
	);
}

void MainWindow::on_brushRadiusSpinBox_valueChanged(int value) {
	visualizationContainer->SetBrushRadius(value);
}

void MainWindow::on_brushRadiusUp() {
	brushRadiusSpinBox->stepUp();
}

void MainWindow::on_brushRadiusDown() {
	brushRadiusSpinBox->stepDown();
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

void MainWindow::createToolBar() {
	// Create tool bar
	QToolBar* toolBar = new QToolBar();
	toolBar->setFloatable(true);
	toolBar->setMovable(true);
	toolBar->setOrientation(Qt::Vertical);

	// Interaction toggle
	QActionGroup* interactionModeGroup = new QActionGroup(this);
	interactionModeGroup->setExclusive(true);

	QAction* actionNavigation = new QAction("N", interactionModeGroup);
	actionNavigation->setToolTip("Navigation mode (space bar)");
	actionNavigation->setCheckable(true);
	actionNavigation->setChecked(visualizationContainer->GetInteractionMode() == NavigationMode);

	QAction* actionEdit = new QAction("E", interactionModeGroup);
	actionEdit->setToolTip("Edit mode (space bar)");
	actionEdit->setCheckable(true);

	QObject::connect(actionNavigation, &QAction::triggered, this, &MainWindow::on_actionNavigation);
	QObject::connect(actionEdit, &QAction::triggered, this, &MainWindow::on_actionEdit);
	QObject::connect(new QShortcut(QKeySequence(Qt::Key_Space), this), &QShortcut::activated, [actionNavigation, actionEdit]() {
		if (actionEdit->isChecked()) {
			actionNavigation->toggle();
			emit(actionNavigation->triggered(true));
		}
		else {
			actionEdit->toggle();
			emit(actionEdit->triggered(true));
		}
	});

	// Rescale toggle
	QAction* actionToggleAutoRescale = new QAction("auto");
	actionToggleAutoRescale->setCheckable(true);
	actionToggleAutoRescale->setChecked(false);

	QObject::connect(actionToggleAutoRescale, &QAction::triggered, this, &MainWindow::on_actionToggleAutoRescale);

	// Add widgets to tool bar
	toolBar->addWidget(createLabel("Mode"));
	toolBar->addAction(actionNavigation);
	toolBar->addAction(actionEdit);
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("2D"));
	toolBar->addAction(createActionIcon(":/icons/icon_overlay.png", "Show overlay (q)", "q", visualizationContainer->GetSliceView()->GetShowLabelSlice(), &MainWindow::on_actionOverlay));
	toolBar->addAction(createActionIcon(":/icons/icon_outline.png", "Show outlines (e)", "e", visualizationContainer->GetSliceView()->GetShowRegionOutlines(), &MainWindow::on_actionOutline));
	toolBar->addAction(createActionIcon(":/icons/icon_rescale_full.png", "Rescale full (=)", "=", &MainWindow::on_actionRescaleFull));
	toolBar->addAction(createActionIcon(":/icons/icon_rescale_partial.png", "Rescale partial (-)", "-", &MainWindow::on_actionRescalePartial));
	toolBar->addAction(actionToggleAutoRescale);
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("3D"));
	toolBar->addAction(createActionIcon(":/icons/icon_smooth_normals.png", "Smooth normals (n)", "n", visualizationContainer->GetVolumeView()->GetSmoothShading(), &MainWindow::on_actionSmoothNormals));
	toolBar->addAction(createActionIcon(":/icons/icon_smooth_surface.png", "Smooth surfaces (s)", "s", visualizationContainer->GetVolumeView()->GetSmoothSurfaces(), &MainWindow::on_actionSmoothSurfaces));
	toolBar->addAction(createActionIcon(":/icons/icon_plane.png", "Show plane (o)", "o", visualizationContainer->GetVolumeView()->GetShowPlane(), &MainWindow::on_actionShowPlane));
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("Filter"));
	toolBar->addAction(createActionIcon(":/icons/icon_filter_regions.png", "Filter regions (b)", "b", visualizationContainer->GetFilterRegions(), &MainWindow::on_actionFilterRegions));
	toolBar->addSeparator();
	toolBar->addWidget(createLabel("Visibility"));
	toolBar->addAction(createActionIcon(":/icons/icon_clear_visibility.png", "Clear visibility (c)", "c", &MainWindow::on_actionClearRegionVisibilities));
	toolBar->addAction(createActionIcon(":/icons/icon_show_plane_regions.png", "Show plane regions (p)", "p", &MainWindow::on_actionShowPlaneRegions));
	toolBar->addAction(createActionIcon(":/icons/icon_show_neighbor_regions.png", "Show neighbors (k)", "k", &MainWindow::on_actionShowNeighborRegions));

	// Need extra logic for interaction mode


	toolBarWidget->layout()->addWidget(toolBar);
}

QAction* MainWindow::createActionIcon(const QString& fileName, const QString& text, const QString& shortcut, void (MainWindow::*slot)()) {
	QAction* action = new QAction(QIcon(fileName), text, this);
	action->setShortcut(QKeySequence(shortcut));
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

QLabel* MainWindow::createLabel(const QString& text, int topMargin, int bottomMargin) {
	QString style = QStringLiteral("color:#999;margin-top:%1px;margin-bottom:%2px").arg(topMargin).arg(bottomMargin);

	QLabel* label = new QLabel(text);
	label->setAlignment(Qt::AlignCenter);
	label->setStyleSheet(style);

	return label;
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