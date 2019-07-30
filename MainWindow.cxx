#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include <vtkGenericOpenGLRenderWindow.h>

#include "VisualizationContainer.h"

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
	visualizationContainer = new VisualizationContainer(qvtkWidgetLeft->GetInteractor(), qvtkWidgetRight->GetInteractor());
}

MainWindow::~MainWindow() {
	// Clean up
	delete visualizationContainer;

	qApp->exit();
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
	if (!visualizationContainer->OpenImageFile(fileName.toStdString())) {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open file.");
		errorMessage.exec();
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

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load data
	if (!visualizationContainer->OpenImageStack(fileNames)) {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open files.");
		errorMessage.exec();
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
	if (!visualizationContainer->OpenSegmentationFile(fileName.toStdString())) {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open file.");
		errorMessage.exec();
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

	// Get file names
	std::vector<std::string> fileNames;

	for (int i = 0; i < fileInfoList.length(); i++) {
		fileNames.push_back(fileInfoList.at(i).absoluteFilePath().toStdString());
	}

	// Load segmentation data
	if (!visualizationContainer->OpenSegmentationStack(fileNames)) {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open files.");
		errorMessage.exec();
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
	if (!visualizationContainer->SaveSegmentationData(fileName.toStdString())) {
		QMessageBox errorMessage;
		errorMessage.setText("Could not save file.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionSegment_Volume_triggered() {
	visualizationContainer->SegmentVolume();
}

void MainWindow::on_actionExit_triggered() {
	// Exit Qt
	qApp->exit();
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
