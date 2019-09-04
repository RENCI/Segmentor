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

#include <vtkDataSetMapper.h>
#include <vtkThreshold.h>
#include <vtkFlyingEdgesPlaneCutter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPointDataToCellData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkExtractEdges.h>
#include <vtkGlyph3DMapper.h>
#include <vtkThresholdPoints.h>
#include <vtkCleanPolyData.h>
#include <vtkAppendFilter.h>
#include <vtkStaticCleanPolyData.h>
#include <vtkDataSetRegionSurfaceFilter.h>
#include <vtkImageToStructuredGrid.h>
#include <vtkAssignAttribute.h>
#include <vtkPointData.h>
#include <vtkImageDataToPointSet.h>
#include <vtkCellCenters.h>
#include <vtkExtractGeometry.h>
#include <vtkAppendFilter.h>

void SlicePipeline::cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	SlicePipeline* pipeline = static_cast<SlicePipeline*>(clientData);

	pipeline->UpdatePlane();
}

SlicePipeline::SlicePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	labels = nullptr;
	labelSlice = nullptr;
	plane = vtkSmartPointer<vtkPlane>::New();

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();
	//renderer->GetActiveCamera()->ParallelProjectionOn();

	style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);

	interactor->GetRenderWindow()->AddRenderer(renderer);
	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Camera callback
	vtkSmartPointer <vtkCallbackCommand> cameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	cameraCallback->SetCallback(cameraChange);
	cameraCallback->SetClientData(this);
	renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cameraCallback);

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

void SlicePipeline::UpdatePlane() {
	vtkCamera* cam = renderer->GetActiveCamera();

	double v[3];
	cam->GetDirectionOfProjection(v);
	double s = -0.01;

	vtkMath::Normalize(v);
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;

	double* o = cam->GetFocalPoint();

	plane->SetOrigin(o[0] + v[0], o[1] + v[1], o[2] + v[2]);
	plane->SetNormal(cam->GetDirectionOfProjection());
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
	property->SetOpacity(0.1);
	
	// Slice
	labelSlice = vtkSmartPointer<vtkImageSlice>::New();
	labelSlice->SetMapper(mapper);
	labelSlice->SetProperty(property);
	labelSlice->PickableOff();
	labelSlice->DragableOff();

	labelSlice->Update();

	// Voxel outlines
	vtkSmartPointer<vtkThresholdPoints> points = vtkSmartPointer<vtkThresholdPoints>::New();
	points->SetUpperThreshold(1);
	points->SetInputDataObject(labels);

	vtkSmartPointer<vtkCubeSource> glyphSource = vtkSmartPointer<vtkCubeSource>::New();

	vtkSmartPointer<vtkGlyph3D> glyph = vtkSmartPointer<vtkGlyph3D>::New();
	glyph->SetSourceConnection(glyphSource->GetOutputPort());
	glyph->ScalingOff();
	glyph->SetInputConnection(points->GetOutputPort());

	vtkSmartPointer<vtkCutter> cut = vtkSmartPointer<vtkCutter>::New();
	cut->SetCutFunction(plane);
	cut->SetInputConnection(glyph->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper2->SetLookupTable(labelColors);
	mapper2->UseLookupTableScalarRangeOn();
	mapper2->SetInputConnection(cut->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->GetProperty()->LightingOff();
	actor->GetProperty()->SetOpacity(0.25);
	actor->PickableOff();
	actor->SetMapper(mapper2);

	renderer->AddActor(actor);
}