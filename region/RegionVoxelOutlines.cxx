#include "RegionVoxelOutlines.h"

#include <vtkActor.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include "Region.h"

RegionVoxelOutlines::RegionVoxelOutlines(Region* inputRegion, double color[3]) {
	region = inputRegion;

	cut = vtkSmartPointer<vtkCutter>::New();
	cut->GenerateTrianglesOff();
	cut->SetInputConnection(region->GetCells());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();
	mapper->SetInputConnection(cut->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->GetProperty()->SetOpacity(0.25);
}
	
RegionVoxelOutlines::~RegionVoxelOutlines() {
	while (actor->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(actor->GetConsumer(0))->RemoveActor(actor);
	}
}

vtkSmartPointer<vtkActor> RegionVoxelOutlines::GetActor() {
	return actor;
}

void RegionVoxelOutlines::SetPlane(vtkPlane* plane) {
	cut->SetCutFunction(plane);
}