#include "SlicePipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithm.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* rwi) {
	this->renderer = vtkSmartPointer<vtkRenderer>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

SlicePipeline::~SlicePipeline() {
}

void SlicePipeline::SetInput(vtkAlgorithm* input) {
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(input->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	renderer->AddActor(actor);
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}