#include "RegionHighlight3D.h"

#include <vtkActor.h>
#include <vtkDiskSource.h>
#include <vtkLookupTable.h>
#include <vtkFollower.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include "Region.h"

RegionHighlight3D::RegionHighlight3D(Region* inputRegion, double color[3]) {
	region = inputRegion;

	disk = vtkSmartPointer<vtkDiskSource>::New();
	disk->SetCircumferentialResolution(32);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();
	mapper->SetInputConnection(disk->GetOutputPort());

	actor = vtkSmartPointer<vtkFollower>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->SetDiffuse(0.0);
	actor->GetProperty()->SetAmbient(1.0);
	actor->GetProperty()->SetSpecular(0.0);
	actor->GetProperty()->SetOpacity(0.5);

	Update();
}

RegionHighlight3D::~RegionHighlight3D() {
	while (actor->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(actor->GetConsumer(0))->RemoveActor(actor);
	}
}

void RegionHighlight3D::SetCamera(vtkCamera* camera) {
	actor->SetCamera(camera);
}

vtkSmartPointer<vtkActor> RegionHighlight3D::GetActor() {
	return actor;
}

void RegionHighlight3D::Update() {
	double r = region->GetLength() / 2;
	double or = r;
	double ir = or / 2;

	disk->SetInnerRadius(ir);
	disk->SetOuterRadius(or);

	actor->SetPosition(region->GetCenter());
}