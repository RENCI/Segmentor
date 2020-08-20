#include "Probe.h"

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkImageData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

Probe::Probe(double probeScale, bool probe3D) : scale(probeScale) {
	vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(source->GetOutputPort());

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetPointSize(2);
	actor->VisibilityOff();
	actor->PickableOff();

	if (probe3D) {
		actor->GetProperty()->SetRepresentationToSurface();
		actor->GetProperty()->EdgeVisibilityOn();
	}
	else {
		actor->GetProperty()->SetRepresentationToWireframe();
	}
}

Probe::~Probe() {
}

void Probe::UpdateData(vtkImageData* data) {
	double spacing[3];
	data->GetSpacing(spacing);

	spacing[0] *= scale;
	spacing[1] *= scale;
	spacing[2] *= scale;

	actor->SetPosition(data->GetCenter());
	actor->SetScale(spacing);
	actor->VisibilityOn();
}

vtkActor* Probe::GetActor() {
	return actor;
}