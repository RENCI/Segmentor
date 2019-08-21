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

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	labels = nullptr;
	labelSlice = nullptr;

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();
	//renderer->GetActiveCamera()->ParallelProjectionOn();

	style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);

	interactor->GetRenderWindow()->AddRenderer(renderer);
	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Colors
	labelColors = lut;

	// Probe
	CreateProbe();
}

SlicePipeline::~SlicePipeline() {
}

void SlicePipeline::SetImageData(vtkImageData* imageData) {
	data = imageData;

	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();

	// Render
	renderer->AddActor(CreateDataSlice(data));
	renderer->ResetCamera();
	Render();
}

void SlicePipeline::SetSegmentationData(vtkImageData* imageLabels) {
	labels = imageLabels;
	CreateLabelSlice(labels);

	// Render
	renderer->AddActor(labelSlice);
	Render();
}

void SlicePipeline::SetShowProbe(bool show) {
	probe->SetVisibility(show);
}

void SlicePipeline::SetProbePosition(double x, double y, double z) {
	vtkCamera* cam = renderer->GetActiveCamera();

	double p1[3] = { x , y, z };
	double p2[3];
	vtkPlane::ProjectPoint(p1, cam->GetFocalPoint(), cam->GetDirectionOfProjection(), p2);

	double* spacing = data->GetSpacing();
	double s = spacing[0];
	
	if (sqrt(vtkMath::Distance2BetweenPoints(p1, p2)) < s) {
		probe->GetProperty()->SetRepresentationToWireframe();
	}
	else {
		probe->GetProperty()->SetRepresentationToPoints();
		probe->GetProperty()->SetPointSize(2);
	}	

	probe->SetPosition(p2);
}

void SlicePipeline::SetCurrentLabel(unsigned short label) {
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
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();
	//mapper->JumpToNearestSliceOn();
	mapper->AutoAdjustImageQualityOff();
	mapper->ResampleToScreenPixelsOn();
	//mapper->SetSlabThickness(10);
	//mapper->SetSlabTypeToSum();
	mapper->SetInputDataObject(data);

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
	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();
	//mapper->JumpToNearestSliceOn();
	mapper->AutoAdjustImageQualityOff();
	mapper->ResampleToScreenPixelsOn();
	//mapper->SetSlabThickness(10);
	//mapper->SetSlabTypeToMin();
	mapper->SetInputDataObject(labels);

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

	labelSlice->Update();
	
	// Below attempts to generate voxels outlines
/*
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdByUpper(1);
	threshold->SetInputDataObject(labels);

	vtkSmartPointer<vtkDataSetMapper> mapper2 = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper2->SetInputConnection(threshold->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetRepresentationToWireframe();
	actor->SetMapper(mapper2);
*/

/*
	vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New(); 
	plane->SetNormal(0, 0, 1);
	plane->SetOrigin(labels->GetDimensions()[0] / 2, labels->GetDimensions()[1] / 2, labels->GetDimensions()[2] / 2);

	vtkSmartPointer<vtkCutter> cut = vtkSmartPointer<vtkCutter>::New();
	cut->SetCutFunction(plane);
	cut->GenerateTrianglesOff();
	cut->SetInputDataObject(labels);

	vtkSmartPointer<vtkDataSetMapper> mapper2 = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper2->SetLookupTable(labelColors);
	mapper2->UseLookupTableScalarRangeOn();
	mapper2->SetInputConnection(cut->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->SetRepresentationToPoints();
	actor->PickableOff();
	actor->SetMapper(mapper2);

	vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
	actor2->GetProperty()->SetRepresentationToWireframe();
	double* p = actor2->GetPosition();
	actor2->SetPosition(p[0] - 0.5, p[1] - 0.5, p[2] - 0.5);
	actor2->PickableOff();
	actor2->SetMapper(mapper2);

	renderer->AddActor(actor);
	renderer->AddActor(actor2);
*/

/*
	vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
	cast->SetOutputScalarTypeToFloat();
	cast->SetInputDataObject(labels);

	vtkSmartPointer<vtkImageDataGeometryFilter> grid = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	grid->OutputTrianglesOn();
	grid->SetInputConnection(cast->GetOutputPort());

	vtkSmartPointer<vtkDataSetMapper> mapper2 = vtkSmartPointer<vtkDataSetMapper>::New();
	mapper2->SetInputConnection(grid->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	//actor->GetProperty()->SetRepresentationToWireframe();
	actor->SetMapper(mapper2);

*/
/*
	//vtkSmartPointer<vtkCubeSource> glyph = vtkSmartPointer<vtkCubeSource>::New();
	//vtkSmartPointer<vtkPlaneSource> glyph = vtkSmartPointer<vtkPlaneSource>::New();
	vtkSmartPointer<vtkSphereSource> glyph = vtkSmartPointer<vtkSphereSource>::New();
	glyph->SetRadius(0.1);

	vtkSmartPointer<vtkGlyph3DMapper> mapper2 = vtkSmartPointer<vtkGlyph3DMapper>::New();
	mapper2->SetSourceConnection(glyph->GetOutputPort());
	mapper2->ScalingOff(); 
	mapper2->SetLookupTable(labelColors);
	mapper2->UseLookupTableScalarRangeOn();
	mapper2->SetInputDataObject(labels);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	//actor->GetProperty()->SetRepresentationToPoints();
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetOpacity(0.25);
	actor->PickableOff();
	actor->SetMapper(mapper2);

	renderer->AddActor(actor);
*/
}