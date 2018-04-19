#include "SlicePipeline.h"

#include "vtkInteractorStyleSlice.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkObject.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* rwi) {
	this->renderer = vtkSmartPointer<vtkRenderer>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

SlicePipeline::~SlicePipeline() {
}

void SlicePipeline::SetInput(vtkImageData* input) {
	// Get image info
	double minValue = input->GetScalarRange()[0];
	double maxValue = input->GetScalarRange()[1];

	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SetInputDataObject(input);
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();

	// Image property
	vtkSmartPointer<vtkImageProperty> property = vtkSmartPointer<vtkImageProperty>::New();
	property->SetInterpolationTypeToNearest();
	property->SetColorWindow(maxValue - minValue);
	property->SetColorLevel(minValue + (maxValue - minValue) / 2);

	// Slice
	vtkSmartPointer<vtkImageSlice> slice = vtkSmartPointer<vtkImageSlice>::New();
	slice->SetMapper(mapper);
	slice->SetProperty(property);

	// Interaction
	vtkSmartPointer<vtkInteractorStyleSlice> style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();

	vtkRenderWindowInteractor* interactor = renderer->GetRenderWindow()->GetInteractor();
	interactor->SetInteractorStyle(style);

	// Render
	renderer->AddActor(slice);
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}