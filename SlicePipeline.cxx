#include "SlicePipeline.h"

#include "vtkInteractorStyleSlice.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkContourFilter.h>
#include <vtkCubeSource.h>
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
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* interactor) {
	labels = nullptr;
	labelSlice = nullptr;

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();
	interactor->GetRenderWindow()->AddRenderer(renderer);
	interactor->SetNumberOfFlyFrames(5);

	// Interaction
	style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);

	interactor->SetInteractorStyle(style);

	CreateProbe();
}

SlicePipeline::~SlicePipeline() {
}

void SlicePipeline::SetImageData(vtkImageData* data) {	
	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();

	// Render
	renderer->AddActor(CreateDataSlice(data));
	renderer->ResetCamera();
	Render();
}

void SlicePipeline::SetSegmentationData(vtkImageData* data) {
	labels = data;
	CreateLabelSlice(labels);

	// Render
	renderer->AddActor(labelSlice);
	Render();
}

void SlicePipeline::SetProbeVisiblity(bool visiblity) {
	probe->SetVisibility(visiblity);
}

void SlicePipeline::SetProbePosition(double x, double y, double z) {
	vtkCamera* cam = renderer->GetActiveCamera();

	double p1[3] = { x , y, z };
	double p2[3];
	vtkPlane::ProjectPoint(p1, cam->GetFocalPoint(), cam->GetDirectionOfProjection(), p2);
	
	if (sqrt(vtkMath::Distance2BetweenPoints(p1, p2)) < 1) {
		probe->GetProperty()->SetOpacity(1);
	}
	else {
		probe->GetProperty()->SetOpacity(0.5);
	}	

	probe->SetPosition(p2);
}

void SlicePipeline::SetLabel(unsigned short label) {
	if (label > 0) {
		double color[3];
		labelSlice->GetProperty()->GetLookupTable()->GetColor(label, color);
		probe->GetProperty()->SetColor(color);
	}
	else {
		probe->GetProperty()->SetColor(1, 1, 1);
	}
}

void SlicePipeline::Render() {
	renderer->GetRenderWindow()->Render();
}

vtkSmartPointer<vtkRenderer> SlicePipeline::GetRenderer() {
	return renderer;
}

vtkSmartPointer<vtkInteractorStyleSlice> SlicePipeline::GetInteractorStyle() {
	return style;
}

void SlicePipeline::CreateProbe() {
	vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(source->GetOutputPort());

	probe = vtkSmartPointer<vtkActor>::New();
	probe->SetMapper(mapper);	
	probe->GetProperty()->SetRepresentationToWireframe();
	probe->GetProperty()->LightingOff();
	probe->VisibilityOff();
	probe->PickableOff();

	renderer->AddActor(probe);
}

void SlicePipeline::UpdateProbe(vtkImageData* data) {
	probe->SetPosition(data->GetCenter());
	probe->SetScale(data->GetSpacing());
}

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

void SlicePipeline::CreateLabelSlice(vtkImageData* labels) {
	// Number of labels
	int maxLabel = labels->GetScalarRange()[1];

	// Colors from ColorBrewer
	const int numColors = 12;
	double colors[numColors][3] = {
		{ 166,206,227 },
		{ 31,120,180 },
		{ 178,223,138 },
		{ 51,160,44 },
		{ 251,154,153 },
		{ 227,26,28 },
		{ 253,191,111 },
		{ 255,127,0 },
		{ 202,178,214 },
		{ 106,61,154 },
		{ 255,255,153 },
		{ 177,89,40 }
	};

	for (int i = 0; i < numColors; i++) {
		for (int j = 0; j < 3; j++) {
			colors[i][j] /= 255.0;
		}
	}

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
		labelColors->SetTableValue(i, c[0], c[1], c[2]);
	}
	labelColors->Build();

	// Image property
	vtkSmartPointer<vtkImageProperty> property = vtkSmartPointer<vtkImageProperty>::New();
	property->SetInterpolationTypeToNearest();
	property->SetLookupTable(labelColors);
	property->UseLookupTableScalarRangeOn();
	property->SetOpacity(0.25);

	// Slice
	labelSlice = vtkSmartPointer<vtkImageSlice>::New();
	labelSlice->SetMapper(mapper);
	labelSlice->SetProperty(property);
	labelSlice->PickableOff();
	labelSlice->DragableOff();
}