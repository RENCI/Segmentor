#include "MainWindow.h"

#include "VolumePipeline.h"
#include "SlicePipeline.h"

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkSmartPointer.h>

// Constructor
MainWindow::MainWindow()
{
	this->setupUi(this);
	
	// Create render windows
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	this->qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	this->qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Create VTK pipelines
	this->volumePipeline = new VolumePipeline(this->qvtkWidgetLeft->GetInteractor());
	this->slicePipeline = new SlicePipeline(this->qvtkWidgetRight->GetInteractor());

	// Set up action signals and slots
	connect(this->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
};

void MainWindow::slotExit()
{
  qApp->exit();
}
