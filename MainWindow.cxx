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

void sliceViewChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	double r = 10;

	vtkCamera* sliceCamera = reinterpret_cast<vtkCamera*>(caller);

	vtkRenderer* volumeRenderer = reinterpret_cast<vtkRenderer*>(clientData);
	vtkCamera* volumeCamera = volumeRenderer->GetActiveCamera();

	volumeCamera->SetFocalPoint(sliceCamera->GetFocalPoint());
	volumeCamera->SetPosition(sliceCamera->GetPosition());
	volumeCamera->SetViewUp(sliceCamera->GetViewUp());

	volumeRenderer->ResetCameraClippingRange();

	volumeCamera->SetClippingRange(volumeCamera->GetDistance() - r, volumeCamera->GetClippingRange()[1]);

	volumeRenderer->GetRenderWindow()->Render();
}

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

	// Callbacks
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
		volumePipeline->SetInput(dataPipeline->GetData(), dataPipeline->GetLabels());
		slicePipeline->SetInput(dataPipeline->GetData(), dataPipeline->GetLabels());
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
