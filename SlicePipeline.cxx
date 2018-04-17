#include "SlicePipeline.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkInteractorStyleImage.h>
#include <vtkObject.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>

// XXX: Probably want to create a new interactor style derived from vtkInteractorStyleImage
int oldX;
int oldY;

void onLeftButtonPress(vtkObject* caller, unsigned long eventId, void* clientData, void* callData) {
	vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(caller);
	style->GetInteractor()->GetEventPosition(oldX, oldY);
}

void onLeftButtonRelease(vtkObject* caller, unsigned long eventId, void* clientData, void* callData) {
	vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(caller);

	int x;
	int y;

	style->GetInteractor()->GetEventPosition(x, y);

	cout << oldX << " " << x << " " << oldY << " " << y << endl;

	if (x == oldX && y == oldY) {
		cout << "CLICK" << endl;
	}
}

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

	// Callbacks
	vtkSmartPointer<vtkCallbackCommand> leftButtonPressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	leftButtonPressCallback->SetCallback(onLeftButtonPress);

	vtkSmartPointer<vtkCallbackCommand> leftButtonReleaseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	leftButtonReleaseCallback->SetCallback(onLeftButtonRelease);

	// Interaction
	vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
	style->SetInteractionModeToImage3D();
	style->AddObserver(vtkCommand::LeftButtonPressEvent, leftButtonPressCallback, 1.0);
	style->AddObserver(vtkCommand::LeftButtonReleaseEvent, leftButtonReleaseCallback, 1.0);

	vtkRenderWindowInteractor* interactor = renderer->GetRenderWindow()->GetInteractor();
	interactor->SetInteractorStyle(style);

	// Render
	renderer->AddActor(slice);
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}