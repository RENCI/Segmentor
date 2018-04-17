#include "MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkSmartPointer.h>

#include "DataPipeline.h"
#include "VolumePipeline.h"
#include "SlicePipeline.h"

// Constructor
MainWindow::MainWindow() {
	// Create the GUI from the Qt Designer file
	setupUi(this);
	
	// Create render windows
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Create VTK pipelines
	dataPipeline = new DataPipeline();
	volumePipeline = new VolumePipeline(this->qvtkWidgetLeft->GetInteractor());
	slicePipeline = new SlicePipeline(this->qvtkWidgetRight->GetInteractor());
};

MainWindow::~MainWindow() {
	// Clean up
	delete dataPipeline;
	delete volumePipeline;
	delete slicePipeline;

	qApp->exit();
}

void MainWindow::on_actionOpen_triggered() {
	// Open a file dialog to read the file
	QString fileName = QFileDialog::getOpenFileName(this,
		"Open Volume",
		"E:/borland/data/BRAIN_I/Sample/",
//		"",
		"All Files (*.*);;VTK XML ImageData Files (*.vti)");

	// Check for file name
	if (fileName == "") {
		return;
	}

	if (dataPipeline->OpenData(fileName.toStdString())) {
		volumePipeline->SetInput(dataPipeline->GetOutput());
		slicePipeline->SetInput(dataPipeline->GetOutput());
	}
	else {
		QMessageBox errorMessage;
		errorMessage.setText("Could not open file.");
		errorMessage.exec();
	}
}

void MainWindow::on_actionExit_triggered() {
	// Exit Qt
	qApp->exit();
}
