#include "MainWindow.h"

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

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	this->qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	this->qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	// Sphere
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> sphereActor =
		vtkSmartPointer<vtkActor>::New();
	sphereActor->SetMapper(sphereMapper);

	// Cube
	vtkSmartPointer<vtkCubeSource> cubeSource =
		vtkSmartPointer<vtkCubeSource>::New();
	cubeSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> cubeMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
	vtkSmartPointer<vtkActor> cubeActor =
		vtkSmartPointer<vtkActor>::New();
	cubeActor->SetMapper(cubeMapper);

	// VTK Renderers
	vtkSmartPointer<vtkRenderer> rendererLeft =
		vtkSmartPointer<vtkRenderer>::New();
	rendererLeft->AddActor(sphereActor);

	vtkSmartPointer<vtkRenderer> rendererRight =
		vtkSmartPointer<vtkRenderer>::New();
	rendererRight->AddActor(cubeActor);

	// VTK/Qt wedded
	this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(rendererLeft);
	this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(rendererRight);

	// Set up action signals and slots
	connect(this->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
};

void MainWindow::slotExit()
{
  qApp->exit();
}
