#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>

#include <vtkGenericOpenGLRenderWindow.h>

#include "VisualizationContainer.h"

#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"

void testEnter(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	std::cout << "HERE" << std::endl;
}


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

	// Test label table
	int n = 10;
	labelTable->setColumnCount(3);
	labelTable->setRowCount(n);

	QStringList headers;
	headers << "Id" << "Color" << "Complete";

	labelTable->setHorizontalHeaderLabels(headers);
	labelTable->verticalHeader()->setVisible(false);

	labelTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	//labelTable->horizontalHeader()->setStretchLastSection(true);

	for (int i = 0; i < n; i++) {
		QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(i + 1));

		QColor color((double)i / n * 255, 0, 0);
		QTableWidgetItem* colorItem = new QTableWidgetItem("");
		colorItem->setBackgroundColor(color);

		QTableWidgetItem* checkItem = new QTableWidgetItem();
		checkItem->setCheckState(Qt::Unchecked);
		
		labelTable->setItem(i, 0, idItem);
		labelTable->setItem(i, 1, colorItem);
		labelTable->setItem(i, 2, checkItem);
	}

	qApp->installEventFilter(this);
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
