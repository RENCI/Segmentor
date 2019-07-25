#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

#include "DataPipeline.h"
#include "VolumePipeline.h"
#include "SlicePipeline.h"

bool firstCallback = true;

void volumeViewChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	if (!firstCallback) return;

	firstCallback = false;

	vtkCamera* volumeCamera = reinterpret_cast<vtkCamera*>(caller);

	vtkRenderer* sliceRenderer = reinterpret_cast<vtkRenderer*>(clientData);
	vtkCamera* sliceCamera = sliceRenderer->GetActiveCamera();

	sliceCamera->SetFocalPoint(volumeCamera->GetFocalPoint());
	sliceCamera->SetPosition(volumeCamera->GetPosition());
	sliceCamera->SetViewUp(volumeCamera->GetViewUp());

	sliceCamera->SetClippingRange(volumeCamera->GetDistance(), volumeCamera->GetClippingRange()[1]);

	sliceRenderer->GetRenderWindow()->Render();

	firstCallback = true;
}

void sliceViewChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	if (!firstCallback) return;

	firstCallback = false;

	double r = 0;

	vtkCamera* sliceCamera = reinterpret_cast<vtkCamera*>(caller);

	vtkRenderer* volumeRenderer = reinterpret_cast<vtkRenderer*>(clientData);
	vtkCamera* volumeCamera = volumeRenderer->GetActiveCamera();

	volumeCamera->SetFocalPoint(sliceCamera->GetFocalPoint());
	volumeCamera->SetPosition(sliceCamera->GetPosition());
	volumeCamera->SetViewUp(sliceCamera->GetViewUp());

	volumeRenderer->ResetCameraClippingRange();

	volumeCamera->SetClippingRange(volumeCamera->GetDistance() - r, volumeCamera->GetClippingRange()[1]);

	volumeRenderer->GetRenderWindow()->Render();



	firstCallback = true;
}

// Constructor
MainWindow::MainWindow() {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	defaultDirectoryKey = "default_directory";

	// Create render windows
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Create VTK pipelines
	dataPipeline = new DataPipeline();
	volumePipeline = new VolumePipeline(this->qvtkWidgetLeft->GetInteractor());
	slicePipeline = new SlicePipeline(this->qvtkWidgetRight->GetInteractor());

	// Callbacks
	vtkSmartPointer <vtkCallbackCommand> volumeCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeCallback->SetCallback(volumeViewChange);
	volumeCallback->SetClientData(slicePipeline->GetRenderer());

	volumePipeline->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, volumeCallback);

	vtkSmartPointer <vtkCallbackCommand> sliceCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceCallback->SetCallback(sliceViewChange);
	sliceCallback->SetClientData(volumePipeline->GetRenderer());

	slicePipeline->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, sliceCallback);
}

MainWindow::~MainWindow() {
	// Clean up
	delete dataPipeline;
	delete volumePipeline;
	delete slicePipeline;

	qApp->exit();
}

void MainWindow::on_actionOpen_Image_File_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		GetDefaultDirectory(),
		"All files (*.*);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(fileName);

	// Load image
	if (dataPipeline->OpenImageFile(fileName.toStdString())) {
		slicePipeline->SetImageData(dataPipeline->GetData());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open file.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionOpen_Image_Stack_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		GetDefaultDirectory(),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(fileName);

	// Get all files in directory
	QFileInfo fileInfo(fileName);
	QDir directory = fileInfo.absoluteDir();
	QFileInfoList fileInfoList = fileInfo.absoluteDir().entryInfoList(QDir::Files, QDir::Name);

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load images
	if (dataPipeline->OpenImageStack(fileNames)) {
		slicePipeline->SetImageData(dataPipeline->GetData());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open files.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionOpen_Segmentation_File_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		GetDefaultDirectory(),
		"All files (*.*);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(fileName);

	// Load segmentation data
	if (dataPipeline->OpenSegmentationFile(fileName.toStdString())) {
		volumePipeline->SetSegmentationData(dataPipeline->GetLabels());
		slicePipeline->SetSegmentationData(dataPipeline->GetLabels());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open file.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionOpen_Segmentation_Stack_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Segmentation Data",
		GetDefaultDirectory(),
		"All files (*.*);;TIFF (*.tif *.tiff)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(fileName);

	// Get all files in directory
	QFileInfo fileInfo(fileName);
	QDir directory = fileInfo.absoluteDir();
	QFileInfoList fileInfoList = fileInfo.absoluteDir().entryInfoList(QDir::Files, QDir::Name);

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load segmentation data
	if (dataPipeline->OpenSegmentationStack(fileNames)) {
		volumePipeline->SetSegmentationData(dataPipeline->GetLabels());
		slicePipeline->SetSegmentationData(dataPipeline->GetLabels());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open files.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionSave_Segmentation_Data_triggered() {
	// Open a file dialog to save the file
	QString fileName = QFileDialog::getSaveFileName(this,
		"Save Segmentation Data", 
		GetDefaultDirectory(),
		"All files (*.*);;TIFF (*.tif);;NIfTI (*.nii);;VTK XML ImageData (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	SetDefaultDirectory(fileName);

	if (!dataPipeline->SaveSegmentationData(fileName.toStdString())) {
		QMessageBox errorMessage;
		errorMessage.setText("Could not save file.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionSegment_Volume_triggered() {
	dataPipeline->SegmentVolume();
	volumePipeline->SetSegmentationData(dataPipeline->GetLabels());
	slicePipeline->SetSegmentationData(dataPipeline->GetLabels());
}

void MainWindow::on_actionExit_triggered() {
	// Exit Qt
	qApp->exit();
}

QString MainWindow::GetDefaultDirectory() {
	QSettings settings;

	return settings.value(defaultDirectoryKey).toString();
}

void MainWindow::SetDefaultDirectory(QString fileName) {
	QFileInfo fileInfo(fileName);

	QSettings settings;
	settings.setValue(defaultDirectoryKey, fileInfo.absoluteDir().absolutePath());
}
