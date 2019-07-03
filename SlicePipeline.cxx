#include "SlicePipeline.h"

#include "vtkInteractorStyleSlice.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkContourFilter.h>
#include <vtkCutter.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkLookupTable.h>
#include <vtkObject.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>

vtkSmartPointer<vtkImageSlice> SlicePipeline::CreateDataSlice(vtkImageData* data) {
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

vtkSmartPointer<vtkImageSlice> SlicePipeline::CreateLabelSlice(vtkImageData* labels) {
	// Number of labels
	int maxLabel = labels->GetScalarRange()[1];

	std::cout << maxLabel << std::endl;

	// Colors from ColorBrewer
	const int numColors = 6;
	double colors[numColors][3] = {
		{ 228, 26, 28 },
		{ 55, 126, 184 },
		{ 77, 175, 74 },
		{ 152, 78, 163 },
		{ 255, 127, 0 },
		{ 255, 255, 51 }
	};

	for (int i = 0; i < numColors; i++) {
		for (int j = 0; j < 3; j++) {
			colors[i][j] /= 255.0;
		}
	}

	double opacity = 0.25;

	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SetInputDataObject(labels);
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();

	// Label colors
	vtkSmartPointer<vtkLookupTable> labelColors = vtkSmartPointer<vtkLookupTable>::New();
	labelColors->SetNumberOfTableValues(maxLabel + 1);
	labelColors->SetRange(0, maxLabel);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
	for (int i = 1; i <= maxLabel; i++) {
		double* c = colors[(i - 1) % numColors];
		labelColors->SetTableValue(i, c[0], c[1], c[2], opacity);
	}
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

/*
vtkSmartPointer<vtkActor> contourSlice(vtkContourFilter* contour) {
	// Plane
	vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
	plane->SetOrigin(0, 0, 100);
	plane->SetNormal(0, 0, 1);

	vtkSmartPointer<vtkCutter> cut = vtkSmartPointer<vtkCutter>::New();
	cut->SetCutFunction(plane);
	cut->SetInputConnection(contour->GetOutputPort());

	// Mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(cut->GetOutputPort());

	// Actor
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	return actor;
}
*/

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* rwi) {
	renderer = vtkSmartPointer<vtkRenderer>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

SlicePipeline::~SlicePipeline() {
}

void SlicePipeline::SetInput(vtkImageData* data, vtkImageData* labels) {	
	// Interaction
	vtkSmartPointer<vtkInteractorStyleSlice> style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);
	style->SetLabels(labels);

	vtkRenderWindowInteractor* interactor = renderer->GetRenderWindow()->GetInteractor();
	interactor->SetInteractorStyle(style);

	// Render
	renderer->AddActor(CreateDataSlice(data));
	renderer->AddActor(CreateLabelSlice(labels));
//	renderer->AddActor(contourSlice(contour));
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* SlicePipeline::GetRenderer() {
	return renderer;
}