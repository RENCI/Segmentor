#include "Brush.h"

#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkExtractVOI.h>
#include <vtkGeometryFilter.h>
#include <vtkImageCast.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageData.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkThreshold.h>
#include <vtkTransform.h>

#include "vtkImageDataCells.h"

Brush::Brush() {
	radius = 1;

	cast = vtkSmartPointer<vtkImageCast>::New();
	cast->SetOutputScalarTypeToUnsignedShort();

	voi = vtkSmartPointer<vtkExtractVOI>::New();
	voi->SetInputConnection(cast->GetOutputPort());

	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->CenterImageOn();
	info->SetInputConnection(voi->GetOutputPort());

	vtkSmartPointer<vtkImageDataCells> cells = vtkSmartPointer<vtkImageDataCells>::New();
	cells->SetInputConnection(info->GetOutputPort());
	
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdByUpper(1);
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
	cast->SetInputDataObject(data);

	UpdateBrush();
}

void Brush::SetRadius(int BrushRadius) {
	radius = BrushRadius;
	
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
	
	voi->SetVOI(0, w, 0, w, 0, 0);
	voi->Update();

	vtkImageData* data = voi->GetOutput();

	for (int x = 0; x <= w; x++) {
		for (int y = 0; y <= w; y++) {
			int xd = c - x;
			int yd = c - y;
			int d2 = xd * xd + yd * yd;

			unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(x, y, 0));

			*p = d2 <= r2 ? 1 : 0;
		}
	}
	
	data->Modified();
}