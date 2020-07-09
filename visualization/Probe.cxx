#include "Probe.h"

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkImageData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

Probe::Probe() {
	vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(source->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetPointSize(2);
	actor->VisibilityOff();
	actor->PickableOff();
}

Probe::~Probe() {
}

void Probe::UpdateData(vtkImageData* data) {
	actor->SetPosition(data->GetCenter());
	actor->SetScale(data->GetSpacing());
	actor->VisibilityOn();
}

vtkActor* Probe::GetActor() {
	return actor;
}