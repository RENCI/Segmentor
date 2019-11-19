#include "RegionOutline.h"

#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkGeometryFilter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include "Region.h"

RegionOutline::RegionOutline(Region* inputRegion, double color[3]) {
	region = inputRegion;

	vtkSmartPointer<vtkGeometryFilter> surface = vtkSmartPointer<vtkGeometryFilter>::New();
	surface->SetInputConnection(region->GetCells());

	cut = vtkSmartPointer<vtkCutter>::New();
	cut->SetInputConnection(surface->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();
	mapper->SetInputConnection(cut->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetLineWidth(2);
	actor->GetProperty()->SetOpacity(0.5);
}
	
RegionOutline::~RegionOutline() {
	while (actor->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(actor->GetConsumer(0))->RemoveActor(actor);
	}
}

vtkSmartPointer<vtkActor> RegionOutline::GetActor() {
	return actor;
}

void RegionOutline::SetPlane(vtkPlane* plane) {
	cut->SetCutFunction(plane);
}