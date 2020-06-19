#include "SliceLocation.h"

#include <vtkActor.h>
#include <vtkBox.h>
#include <vtkCamera.h>
#include <vtkClipPolyData.h>
#include <vtkAxesActor.h>
#include <vtkCubeSource.h>
#include <vtkCutter.h>
#include <vtkImageData.h>
#include <vtkLineSource.h>
#include <vtkMath.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

SliceLocation::SliceLocation(vtkRenderer* ren) {
	sliceColor[0] = 0;
	sliceColor[1] = 0;
	sliceColor[2] = 0.75;

	outlineColor[0] = 0.5;
	outlineColor[1] = 0.5;
	outlineColor[2] = 0.5;

	length = 0;

	renderer = ren;
	renderer->GetActiveCamera()->Azimuth(-30.0);
	renderer->GetActiveCamera()->Elevation(20.0);

	CreateOutline();
	CreateCorners();
	CreatePlane();
	CreatePlaneInset();
	CreateViewDirection();
	CreateAxes();
}

SliceLocation::~SliceLocation() {
}

void SliceLocation::UpdateData(vtkImageData* data) {
	if (data == nullptr) {
		vtkPropCollection* props = renderer->GetViewProps();
		props->InitTraversal();
		for (int i = 0; i < props->GetNumberOfItems(); i++) {
			props->GetNextProp()->VisibilityOff();
		}

		return;
	}

	double* bounds = data->GetBounds();
	length = data->GetLength();
	
	double minAxis = 0;
	for (int i = 0; i < 3; i++) {
		double d = bounds[i * 2 + 1] - bounds[i * 2];
		if (d > minAxis) minAxis = d;
	}

	outline->SetInputDataObject(data);
	corners->SetInputDataObject(data);

	planeCube->SetBounds(bounds);
	box->SetBounds(bounds);

	sphereActor->SetScale(minAxis / 20);

	double a = minAxis / 3;
	axes->SetTotalLength(a, a, a);

	vtkPropCollection* props = renderer->GetViewProps();
	props->InitTraversal();
	for (int i = 0; i < props->GetNumberOfItems(); i++) {
		props->GetNextProp()->VisibilityOn();

		if (i == 0) renderer->ResetCamera();
	}

	renderer->ResetCameraClippingRange();
}

void SliceLocation::UpdateView(vtkCamera* camera, vtkPlane* cutPlane) {
	double* o = camera->GetFocalPoint();
	double* v = camera->GetDirectionOfProjection();

	// Plane
	planeCutter->SetCutFunction(cutPlane);
	
	// Plane inset
	double distance = camera->GetDistance();
	double x = distance * tan(vtkMath::RadiansFromDegrees(camera->GetViewAngle()));
	double y = x;

	insetPlane->SetOrigin(-x / 2, -y / 2, 0.0);
	insetPlane->SetPoint1(x / 2, -y / 2, 0.0);
	insetPlane->SetPoint2(-x / 2, y / 2, 0.0);

	insetPlane->SetCenter(camera->GetFocalPoint());
	insetPlane->SetNormal(camera->GetViewPlaneNormal());

	// View direction
	double s = length * 0.1;
	lineSource->SetPoint1(o);
	lineSource->SetPoint2(o[0] - v[0] * s, o[1] - v[1] * s, o[2] - v[2] * s);

	sphereActor->SetPosition(o);
}

void SliceLocation::CreateOutline() {
	outline = vtkSmartPointer<vtkOutlineFilter>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(outline->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetColor(outlineColor);
	actor->GetProperty()->SetOpacity(0.25);
	actor->GetProperty()->LightingOff();
	actor->SetMapper(mapper);
	actor->VisibilityOff();

	renderer->AddActor(actor);
}

void SliceLocation::CreateCorners() {
	corners = vtkSmartPointer<vtkOutlineCornerFilter>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(corners->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetColor(outlineColor);
	actor->GetProperty()->LightingOff();
	actor->SetMapper(mapper);
	actor->VisibilityOff();

	renderer->AddActor(actor);
}

void SliceLocation::CreatePlane() {
	planeCube = vtkSmartPointer<vtkCubeSource>::New();

	planeCutter = vtkSmartPointer<vtkCutter>::New();
	planeCutter->SetCutFunction(vtkSmartPointer<vtkPlane>::New());
	planeCutter->SetInputConnection(planeCube->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(planeCutter->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetColor(sliceColor);
	actor->GetProperty()->LightingOff();
	actor->SetMapper(mapper);
	actor->VisibilityOff();

	renderer->AddActor(actor);
}

void SliceLocation::CreatePlaneInset() {
	insetPlane = vtkSmartPointer<vtkPlaneSource>::New();
	insetPlane->SetXResolution(100);
	insetPlane->SetYResolution(100);

	box = vtkSmartPointer<vtkBox>::New();

	vtkSmartPointer<vtkClipPolyData> clip = vtkSmartPointer<vtkClipPolyData>::New();
	clip->SetClipFunction(box);
	clip->InsideOutOn();
	clip->SetInputConnection(insetPlane->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(clip->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetOpacity(0.5);
	actor->GetProperty()->SetColor(sliceColor);
	actor->GetProperty()->LightingOff();
	actor->SetMapper(mapper);
	actor->VisibilityOff();

	renderer->AddActor(actor);
}

void SliceLocation::CreateViewDirection() {
	lineSource = vtkSmartPointer<vtkLineSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(lineSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetColor(sliceColor);
	actor->GetProperty()->LightingOff();
	actor->SetMapper(mapper);
	actor->VisibilityOff();

	renderer->AddActor(actor);

	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper2->SetInputConnection(sphereSource->GetOutputPort());

	sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->GetProperty()->SetColor(sliceColor);
	sphereActor->GetProperty()->LightingOff();
	sphereActor->SetMapper(mapper2);
	sphereActor->VisibilityOff();

	renderer->AddActor(sphereActor);
}

void SliceLocation::CreateAxes() {
	axes = vtkAxesActor::New();
	axes->VisibilityOff();
	axes->GetXAxisShaftProperty()->SetLineWidth(2);

	renderer->AddActor(axes);
}