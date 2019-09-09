#include "SliceLocation.h"

#include <vtkActor.h>
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkClipPolyData.h>
#include <vtkCubeAxesActor.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkFrustumSource.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

SliceLocation::SliceLocation(vtkRenderer* ren) {
	renderer = ren;
	renderer->GetActiveCamera()->Azimuth(-30.0);

	CreateAxes();
	CreatePlane();
	CreatePosition();
	CreateCameraActor();
}

SliceLocation::~SliceLocation() {
}

void SliceLocation::UpdateData(vtkImageData* data) {
	double* bounds = data->GetBounds();
	double width = bounds[1] - bounds[0];
	double length = data->GetLength();

	axes->SetBounds(bounds);
	axes->VisibilityOn();

	cubeSource->SetBounds(bounds);
	plane->VisibilityOn();

	//planeSource->SetOrigin(bounds[0] - length, bounds[2] - length, 0.0);
	//planeSource->SetPoint1(bounds[1] + length, bounds[2] - length, 0.0);
	//planeSource->SetPoint2(bounds[0] - length, bounds[3] + length, 0.0);
	//planeSource->SetOrigin(bounds[0], bounds[2], 0.0);
	//planeSource->SetPoint1(bounds[1], bounds[2], 0.0);
	//planeSource->SetPoint2(bounds[0], bounds[3], 0.0);
	visibleActor->VisibilityOn();

	position->SetScale(length / 20);
	position->VisibilityOn();

	renderer->ResetCamera();
	renderer->ResetCameraClippingRange();
}

void SliceLocation::UpdateView(vtkCamera* camera, vtkPlane* cutPlane) {
	double* o = camera->GetFocalPoint();
	double* up = camera->GetViewUp();
	double* v = camera->GetDirectionOfProjection();

	double right[3];
	vtkMath::Cross(up, v, right);

	double p1[3] = { 
		o[0] + up[0], o[1] + up[1], o[2] + up[2]
	};

	double p2[3] = {
		o[0] + right[0], o[1] + right[1], o[2] + right[2]
	};


	cutter->SetCutFunction(cutPlane);
	
	//planeSource->SetCenter(camera->GetFocalPoint());
	//planeSource->SetNormal(camera->GetDirectionOfProjection());

	//plane->SetPosition(camera->GetFocalPoint());
	//plane->SetOrientation(camera->GetOrientation());

	
	double distance = camera->GetDistance();
	double x = distance * tan(vtkMath::RadiansFromDegrees(camera->GetViewAngle()));
	double y = x;

	//planeSource->SetCenter(camera->GetFocalPoint());
	planeSource->SetNormal(camera->GetDirectionOfProjection());

	visibleActor->SetPosition(camera->GetFocalPoint());
	//visibleActor->SetOrientation(camera->GetOrientation());
	visibleActor->SetScale(x, y, 1);

	/*
	double coeffs[24];
	camera->GetFrustumPlanes(1.0, coeffs);
	vtkSmartPointer<vtkPlanes> planes = vtkSmartPointer<vtkPlanes>::New();
	planes->SetFrustumPlanes(coeffs);
	visibleMapper->SetClippingPlanes(planes);
	*/
	/*
	vtkSmartPointer<vtkPlane> testPlane = vtkSmartPointer<vtkPlane>::New();
	testPlane->SetOrigin(o);
	testPlane->SetNormal(0, 1, 0);
	visibleMapper->RemoveAllClippingPlanes();
	visibleMapper->AddClippingPlane(testPlane);
	*/

	position->SetPosition(camera->GetPosition());

	renderer->ResetCamera();
}

void SliceLocation::CreateAxes() {
	axes = vtkSmartPointer<vtkCubeAxesActor>::New();
	axes->XAxisLabelVisibilityOff();
	axes->YAxisLabelVisibilityOff();
	axes->ZAxisLabelVisibilityOff();
	axes->SetFlyModeToStaticEdges();
	axes->SetCamera(renderer->GetActiveCamera()); 
	axes->VisibilityOff();
	
	renderer->AddActor(axes);
}

void SliceLocation::CreatePlane() {
	cubeSource = vtkSmartPointer<vtkCubeSource>::New();

	cutter = vtkSmartPointer<vtkCutter>::New();
	cutter->SetInputConnection(cubeSource->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(cutter->GetOutputPort());

	plane = vtkSmartPointer<vtkActor>::New();
	plane->GetProperty()->SetOpacity(0.5);
	plane->GetProperty()->SetRepresentationToWireframe();
	plane->GetProperty()->LightingOff();
	plane->SetMapper(mapper);
	plane->VisibilityOff();

	renderer->AddActor(plane);
}

void SliceLocation::CreatePosition() {
	vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(sphere->GetOutputPort());

	position = vtkSmartPointer<vtkActor>::New();
	position->SetMapper(mapper);
	position->VisibilityOff();

	renderer->AddActor(position);
}

void SliceLocation::CreateCameraActor() {
	planeSource = vtkSmartPointer<vtkPlaneSource>::New();
	planeSource->SetXResolution(10);
	planeSource->SetYResolution(10);
	planeSource->SetOrigin(-0.5, -0.5, 0.0);
	planeSource->SetPoint1(0.5, -0.5, 0.0);
	planeSource->SetPoint2(-0.5, 0.5, 0.0);

	visibleMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	visibleMapper->SetInputConnection(planeSource->GetOutputPort());

	visibleActor = vtkSmartPointer<vtkActor>::New();
	visibleActor->SetMapper(visibleMapper);
	visibleActor->VisibilityOff();

	renderer->AddActor(visibleActor);
}