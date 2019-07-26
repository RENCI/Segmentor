#include "SlicePipeline.h"

#include "vtkInteractorStyleSlice.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
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

#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <vtkProperty.h>

#include <vtkDataSetMapper.h>

#include <vtkCubeSource.h>

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* rwi) {
	labels = nullptr;
	label = 0;
	labelSlice = nullptr;

	renderer = vtkSmartPointer<vtkRenderer>::New();
	rwi->GetRenderWindow()->AddRenderer(renderer);
	rwi->SetNumberOfFlyFrames(5);

	plane = vtkSmartPointer<vtkPlane>::New();

	// Interaction
	style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);
	style->SetSlicePipeline(this);

	vtkRenderWindowInteractor* interactor = renderer->GetRenderWindow()->GetInteractor();
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
	renderer->ResetCamera();
	Render();
}

void SlicePipeline::ShowProbe(bool show) {
	probe->SetVisibility(show);
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

void SlicePipeline::PickLabel(int x, int y, int z) {
	// Toggle the label
	label = static_cast<unsigned short*>(this->labels->GetScalarPointer(x, y, z))[0];

	if (label > 0) {
		double color[3];
		labelSlice->GetProperty()->GetLookupTable()->GetColor(label, color);
		probe->GetProperty()->SetColor(color);
	}
	else {
		probe->GetProperty()->SetColor(1, 1, 1);
	}
}

void SlicePipeline::Paint(int x, int y, int z) {
	// Toggle the label
	unsigned short* p = static_cast<unsigned short*>(this->labels->GetScalarPointer(x, y, z));
	p[0] = label;
	labels->Modified();	
	Render();
}

void SlicePipeline::Render() {
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* SlicePipeline::GetRenderer() {
	return renderer;
}

vtkInteractorStyleSlice* SlicePipeline::GetInteractorStyle() {
	return style;
}

vtkPlane* SlicePipeline::GetPlane() {
	return plane;
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
	labelSlice = vtkSmartPointer<vtkImageSlice>::New();
	labelSlice->SetMapper(mapper);
	labelSlice->SetProperty(property);
	labelSlice->PickableOff();
	labelSlice->DragableOff();
}

vtkSmartPointer<vtkActor> SlicePipeline::CreateLabelSlice2(vtkImageData* labels) {
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

	double opacity = 0.25;

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

	// Mapper
	vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper->UseLookupTableScalarRangeOn();
	mapper->SetInputDataObject(labels);

	// Actor
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	return actor;
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