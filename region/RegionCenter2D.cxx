#include "RegionCenter2D.h"

#include <vtkActor.h>
#include <vtkSphereSource.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include "Region.h"

RegionCenter2D::RegionCenter2D(Region* inputRegion, double color[3]) {
	region = inputRegion;

	sphere = vtkSmartPointer<vtkSphereSource>::New();
	sphere->SetThetaResolution(16);
	sphere->SetPhiResolution(3);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();
	mapper->SetInputConnection(sphere->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->SetDiffuse(0.0);
	actor->GetProperty()->SetAmbient(1.0);
	actor->GetProperty()->SetSpecular(0.0);
	actor->PickableOff();
	actor->VisibilityOff();

	Update(region->GetCenter()[2]);
}

RegionCenter2D::~RegionCenter2D() {
	while (actor->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(actor->GetConsumer(0))->RemoveActor(actor);
	}
}

vtkSmartPointer<vtkActor> RegionCenter2D::GetActor() {
	return actor;
}

void RegionCenter2D::Update(double z) {
	double r = 1;

	double numSlices = 4.0;

	double* c = region->GetCenter();

	double d = fabs(z - c[2]);

	double s = (numSlices - d) / numSlices;
	if (s < 0) s = 0;

	actor->SetPosition(c[0], c[1], z);
	actor->SetScale(r * s);
}