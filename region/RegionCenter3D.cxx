#include "RegionCenter3D.h"

#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include "Region.h"

RegionCenter3D::RegionCenter3D(Region* inputRegion, double color[3]) {
	region = inputRegion;

	sphere = vtkSmartPointer<vtkSphereSource>::New();
	sphere->SetThetaResolution(16);
	sphere->SetPhiResolution(16);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();
	mapper->SetInputConnection(sphere->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->SetScale(1.25);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->SetDiffuse(1.0);
	actor->GetProperty()->SetAmbient(0.0);
	actor->GetProperty()->SetSpecular(0.0);
	actor->VisibilityOff();

	Update();
}

RegionCenter3D::~RegionCenter3D() {
	while (actor->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(actor->GetConsumer(0))->RemoveActor(actor);
	}
}

vtkSmartPointer<vtkActor> RegionCenter3D::GetActor() {
	return actor;
}

void RegionCenter3D::Update() {
	actor->SetPosition(region->GetCenter());
}