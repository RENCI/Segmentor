#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QIcon>
#include <QStyle>
#include <QPushButton>
#include <QToolBar>

#include <vtkGenericOpenGLRenderWindow.h>

#include "VisualizationContainer.h"
#include "RegionCollection.h"
#include "RegionTable.h"
#include "RegionMetadataIO.h"
#include "InteractionEnums.h"
#include "SliceView.h"
#include "VolumeView.h"

#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"

// Constructor
MainWindow::MainWindow() {
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
	CreateToolBar();

	// Create region table
	regionTable = new RegionTable();
	regionTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	regionTableContainer->layout()->addWidget(regionTable);

	QObject::connect(regionTable, &RegionTable::regionDone, this, &MainWindow::on_regionDone);
	QObject::connect(regionTable, &RegionTable::removeRegion, this, &MainWindow::on_removeRegion);

	qApp->installEventFilter(this);
}

MainWindow::~MainWindow() {
	// Clean up
	delete visualizationContainer;

	qApp->exit();
}

void MainWindow::updateRegions(RegionCollection* regions) {
	regionTable->update(regions);
}

void MainWindow::updateRegion(Region* region) {
	regionTable->update(region);
}

void MainWindow::highlightRegion(unsigned short label) {
	regionTable->highlight(label);
}

void MainWindow::on_actionOpen_Image_File_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		GetDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultImageDirectoryKey, fileName);

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
		GetDefaultDirectory(defaultImageDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultImageDirectoryKey, fileName);

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
	VisualizationContainer::FileErrorCode errorCode = visualizationContainer->OpenImageFile(fileName.toStdString());

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
		GetDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

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
		GetDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

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

void MainWindow::on_actionSave_Segmentation_Data_triggered() {
	// Open a file dialog to save the file
	QString fileName = QFileDialog::getSaveFileName(this,
		"Save Segmentation Data",
		GetDefaultDirectory(defaultSegmentationDirectoryKey),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(defaultSegmentationDirectoryKey, fileName);

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

void MainWindow::on_actionSegment_Volume_triggered() {
	visualizationContainer->SegmentVolume();
}

void MainWindow::on_actionExit_triggered() {
	// Exit Qt
	qApp->exit();
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

void MainWindow::on_actionVoxels(bool checked) {
	visualizationContainer->GetSliceView()->ShowVoxelOutlines(checked);
}

void MainWindow::on_actionOutline(bool checked) {
	visualizationContainer->GetSliceView()->ShowRegionOutlines(checked);
}

void MainWindow::on_actionSmoothNormals(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothShading(checked);
}

void MainWindow::on_actionSmoothSurfaces(bool checked) {
	visualizationContainer->GetVolumeView()->SetSmoothSurfaces(checked);
}

void MainWindow::on_regionDone(int label, bool done) {
	visualizationContainer->SetRegionDone((unsigned short)label, done);
}
void MainWindow::on_removeRegion(int label) {
	visualizationContainer->RemoveRegion((unsigned short)label);
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

QString MainWindow::GetDefaultDirectory(QString key) {
	QSettings settings;

	return settings.value(key).toString();
}

void MainWindow::SetDefaultDirectory(QString key, QString fileName) {
	QFileInfo fileInfo(fileName);

	QSettings settings;
	settings.setValue(key, fileInfo.absoluteDir().absolutePath());
}

void MainWindow::CreateToolBar() {
	QToolBar* toolBar = new QToolBar();
	toolBar->setFloatable(true);
	toolBar->setMovable(true);
	toolBar->setOrientation(Qt::Vertical);

	QActionGroup* interactionModeGroup = new QActionGroup(this);
	interactionModeGroup->setExclusive(true);

	QAction* actionNavigation = new QAction("N", interactionModeGroup);
	actionNavigation->setCheckable(true);
	actionNavigation->setChecked(visualizationContainer->GetInteractionMode() == NavigationMode);

	QAction* actionEdit = new QAction("E", interactionModeGroup);
	actionEdit->setCheckable(true);

	QAction* actionOverlay = new QAction(QIcon(":/icons/icon_overlay.svg"), "Show overlay", this);
	actionOverlay->setCheckable(true);
	actionOverlay->setChecked(visualizationContainer->GetSliceView()->GetShowLabelSlice());

	QAction* actionVoxels = new QAction(QIcon(":/icons/icon_voxels.svg"), "Show voxels", this);
	actionVoxels->setCheckable(true);
	actionVoxels->setChecked(visualizationContainer->GetSliceView()->GetShowVoxelOutlines());

	QAction* actionOutline = new QAction(QIcon(":/icons/icon_outline.svg"), "Show outlines", this);
	actionOutline->setCheckable(true);
	actionOutline->setChecked(visualizationContainer->GetSliceView()->GetShowRegionOutlines());

	QAction* actionSmoothNormals = new QAction(QIcon(":/icons/icon_smooth_normals.svg"), "Smooth normals", this);
	actionSmoothNormals->setCheckable(true);
	actionSmoothNormals->setChecked(visualizationContainer->GetVolumeView()->GetSmoothShading());

	QAction* actionSmoothSurfaces = new QAction(QIcon(":/icons/icon_smooth_surface.svg"), "Smooth surfaces", this);
	actionSmoothSurfaces->setCheckable(true);
	actionSmoothSurfaces->setChecked(visualizationContainer->GetVolumeView()->GetSmoothSurfaces());

	toolBar->addAction(actionNavigation);
	toolBar->addAction(actionEdit);
	toolBar->addSeparator();
	toolBar->addAction(actionOverlay);
	toolBar->addAction(actionVoxels);
	toolBar->addAction(actionOutline);
	toolBar->addSeparator();
	toolBar->addAction(actionSmoothNormals);
	toolBar->addAction(actionSmoothSurfaces);

	QObject::connect(actionNavigation, &QAction::triggered, this, &MainWindow::on_actionNavigation);
	QObject::connect(actionEdit, &QAction::triggered, this, &MainWindow::on_actionEdit);
	QObject::connect(actionOverlay, &QAction::triggered, this, &MainWindow::on_actionOverlay);
	QObject::connect(actionVoxels, &QAction::triggered, this, &MainWindow::on_actionVoxels);
	QObject::connect(actionOutline, &QAction::triggered, this, &MainWindow::on_actionOutline);
	QObject::connect(actionSmoothNormals, &QAction::triggered, this, &MainWindow::on_actionSmoothNormals);
	QObject::connect(actionSmoothSurfaces, &QAction::triggered, this, &MainWindow::on_actionSmoothSurfaces);

	toolBarWidget->layout()->addWidget(toolBar);
}