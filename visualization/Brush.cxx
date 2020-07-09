#include "Brush.h"

#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkExtractVOI.h>
#include <vtkGeometryFilter.h>
#include <vtkImageData.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>

#include "vtkImageDataCells.h"

Brush::Brush() {
	radius = 1;

	voi = vtkSmartPointer<vtkExtractVOI>::New();

	cells = vtkSmartPointer<vtkImageDataCells>::New();
	cells->SetInputConnection(voi->GetOutputPort());
	
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdBetween(1, 1);
	threshold->SetInputConnection(cells->GetOutputPort());

	vtkSmartPointer<vtkGeometryFilter> geometry = vtkSmartPointer<vtkGeometryFilter>::New();
	geometry->SetInputConnection(threshold->GetOutputPort());

	vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
	plane->SetNormal(0, 0, 1);

	vtkSmartPointer<vtkCutter> cut = vtkSmartPointer<vtkCutter>::New();
	cut->SetCutFunction(plane);
	cut->SetInputConnection(geometry->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();
	mapper->SetInputConnection(cut->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1, 1, 1);
	actor->GetProperty()->SetOpacity(0.25);
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetLineWidth(2);
	actor->VisibilityOff();
	actor->PickableOff();
}

Brush::~Brush() {
}

void Brush::UpdateData(vtkImageData* data) {
	voi->SetInputDataObject(data);

	UpdateBrush();
}

void Brush::SetRadius(int brushRadius) {
	radius = brushRadius;
	
	UpdateBrush();
}

int Brush::GetRadius() {
	return radius;
}

vtkActor* Brush::GetActor() {
	return actor;
}

void Brush::UpdateBrush() {
	if (!voi->GetInput() || radius <= 1) return;
	
	int w = radius * 2 - 2;
	int c = radius - 1;
	int r2 = c * c;
	
	voi->SetVOI(0, w, 0, w, 0, 1);
	voi->Update();

	vtkImageData* data = voi->GetOutput();

	for (int x = 0; x <= w; x++) {
		for (int y = 0; y <= w; y++) {
			int xd = c - x;
			int yd = c - y;
			int d2 = xd * xd + yd * yd;

			unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(x, y, 0));

			*p = d2 <= r2 ? 1 : 0;

			p = static_cast<unsigned short*>(data->GetScalarPointer(x, y, 1));

			*p = 0;
		}
	}

	double spacing[3];
	data->GetSpacing(spacing);
	data->SetOrigin(-c * spacing[0], -c * spacing[1], -spacing[2] / 2);

	data->Modified();
}