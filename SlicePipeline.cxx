#include "SlicePipeline.h"

#include "vtkInteractorStyleSlice.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>
#include <vtkObject.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>

vtkSmartPointer<vtkImageSlice> DataSlice(vtkImageData* data) {
	// Get image info
	double minValue = data->GetScalarRange()[0];
	double maxValue = data->GetScalarRange()[1];

	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SetInputDataObject(data);
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

	return slice;
}

vtkSmartPointer<vtkImageSlice> LabelSlice(vtkImageData* labels) {
	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SetInputDataObject(labels);
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();

	// Label colors
	vtkSmartPointer<vtkLookupTable> labelColors = vtkSmartPointer<vtkLookupTable>::New();
	labelColors->SetNumberOfTableValues(2);
	labelColors->SetRange(0.0, 1.0);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0, 0.0); //label 0 is transparent
	labelColors->SetTableValue(1, 0.0, 1.0, 0.0, 1.0); //label 1 is opaque and green
	labelColors->Build();

	// Image property
	vtkSmartPointer<vtkImageProperty> property = vtkSmartPointer<vtkImageProperty>::New();
	property->SetInterpolationTypeToNearest();
	property->SetLookupTable(labelColors);
	property->UseLookupTableScalarRangeOn();

	// Slice
	vtkSmartPointer<vtkImageSlice> slice = vtkSmartPointer<vtkImageSlice>::New();
	slice->SetMapper(mapper);
	slice->SetProperty(property);
	slice->PickableOff();
	slice->DragableOff();

	return slice;
}

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* rwi) {
	this->renderer = vtkSmartPointer<vtkRenderer>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

SlicePipeline::~SlicePipeline() {
}

void SlicePipeline::SetInput(vtkImageData* data, vtkImageData* labels) {	
	// Interaction
	vtkSmartPointer<vtkInteractorStyleSlice> style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);

	vtkRenderWindowInteractor* interactor = renderer->GetRenderWindow()->GetInteractor();
	interactor->SetInteractorStyle(style);

	// Render
	renderer->AddActor(DataSlice(data));
	renderer->AddActor(LabelSlice(labels));
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}