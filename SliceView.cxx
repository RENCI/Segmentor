#include "SliceView.h"

#include "vtkInteractorStyleSlice.h"
#include "vtkImageDataCells.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
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
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include "InteractionEnums.h"
#include "Region.h"
#include "RegionOutline.h"
#include "RegionVoxelOutlines.h"
#include "RegionCollection.h"
#include "SliceLocation.h"

void SliceView::cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	SliceView* pipeline = static_cast<SliceView*>(clientData);

	pipeline->UpdatePlane();
}

SliceView::SliceView(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	showRegionOutlines = true;
	showVoxelOutlines = false;
	filterRegion = false;

	data = nullptr;
	labels = nullptr;

	regions = nullptr;
	currentRegion = nullptr;

	labelColors = lut;

	// Create slice pipeline
	plane = vtkSmartPointer<vtkPlane>::New();

	labelSlice = vtkSmartPointer<vtkImageSlice>::New();

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->SetLayer(0);

	labelSliceRenderer = vtkSmartPointer<vtkRenderer>::New();
	labelSliceRenderer->SetLayer(1);
	labelSliceRenderer->InteractiveOff();
	labelSliceRenderer->SetActiveCamera(renderer->GetActiveCamera());

	voxelOutlinesRenderer = vtkSmartPointer<vtkRenderer>::New();
	voxelOutlinesRenderer->SetLayer(2);
	voxelOutlinesRenderer->InteractiveOff();
	voxelOutlinesRenderer->SetActiveCamera(renderer->GetActiveCamera());

	regionOutlinesRenderer = vtkSmartPointer<vtkRenderer>::New();
	regionOutlinesRenderer->SetLayer(2);
	regionOutlinesRenderer->InteractiveOff();
	regionOutlinesRenderer->SetActiveCamera(renderer->GetActiveCamera());

	vtkSmartPointer<vtkRenderer> sliceLocationRenderer = vtkSmartPointer<vtkRenderer>::New();
	sliceLocationRenderer->SetLayer(3);
	sliceLocationRenderer->InteractiveOff();
	sliceLocationRenderer->SetViewport(0.8, 0.0, 1.0, 0.2);

	style = vtkSmartPointer<vtkInteractorStyleSlice>::New();
	style->SetInteractionModeToImage3D();
	style->SetCurrentImageNumber(0);

	interactor->GetRenderWindow()->SetNumberOfLayers(4);
	interactor->GetRenderWindow()->AddRenderer(renderer);
	interactor->GetRenderWindow()->AddRenderer(labelSliceRenderer);
	interactor->GetRenderWindow()->AddRenderer(voxelOutlinesRenderer);
	interactor->GetRenderWindow()->AddRenderer(regionOutlinesRenderer);
	interactor->GetRenderWindow()->AddRenderer(sliceLocationRenderer);
	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Slices
	CreateSlice();
	CreateLabelSlice();

	// Camera callback
	vtkSmartPointer <vtkCallbackCommand> cameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	cameraCallback->SetCallback(cameraChange);
	cameraCallback->SetClientData(this);
	renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cameraCallback);

	// Probe
	CreateProbe();

	// Slice location
	sliceLocation = new SliceLocation(sliceLocationRenderer);

	// Interaction mode label
	CreateInteractionModeLabel();
}

SliceView::~SliceView() {
	delete sliceLocation;
}

void SliceView::Reset() {
	data = nullptr;
	labels = nullptr;

	SetCurrentRegion(nullptr);

	//slice->VisibilityOff();
	//labelSlice->VisibilityOff();
	probe->VisibilityOff();
	sliceLocation->UpdateData(nullptr);
	interactionModeLabel->VisibilityOff();
}

void SliceView::SetImageData(vtkImageData* imageData) {
	// Update slice
	data = imageData;
	UpdateSlice();

	// Update probe
	UpdateProbe(data);

	// Update axes
	sliceLocation->UpdateData(data);

	// Turn on interaction mode
	interactionModeLabel->VisibilityOn();

	// Reset camera
	ResetCamera();
}

void SliceView::SetSegmentationData(vtkImageData* imageLabels, RegionCollection* newRegions) {
	labels = imageLabels;
	UpdateLabelSlice();

	regions = newRegions;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		AddRegionActors(regions->Get(it));
	}

	FilterRegions();
}

void SliceView::AddRegion(Region* region) {
	AddRegionActors(region);

	FilterRegions();
}

void SliceView::AddRegionActors(Region* region) {
	RegionOutline* outline = region->GetOutline();
	outline->SetPlane(plane);
	regionOutlinesRenderer->AddActor(outline->GetActor());

	RegionVoxelOutlines* voxelOutlines = region->GetVoxelOutlines();
	voxelOutlines->SetPlane(plane);
	voxelOutlinesRenderer->AddActor(voxelOutlines->GetActor());
}

void SliceView::SetCurrentRegion(Region* region) {
	currentRegion = region;

	if (currentRegion) {
		const double* color = region->GetColor();
		probe->GetProperty()->SetColor(color[0], color[1], color[2]);
	}
	else {
		probe->GetProperty()->SetColor(1, 1, 1);
	}

	FilterRegions();
}

void SliceView::SetShowProbe(bool show) {
	probe->SetVisibility(show);
}

void SliceView::SetProbePosition(double x, double y, double z) {
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

void SliceView::SetInteractionMode(enum InteractionMode mode) {
	std::string s = mode == NavigationMode ? "Navigation mode" : "Edit mode";
	interactionModeLabel->SetInput(s.c_str());

	style->SetMode(mode);
}

bool SliceView::GetShowLabelSlice() {
	return labelSlice->GetVisibility();
}

void SliceView::ShowLabelSlice(bool show) {
	labelSlice->SetVisibility(show);

	Render();
}

void SliceView::ToggleLabelSlice() {
	ShowLabelSlice(!labelSlice->GetVisibility());
}

bool SliceView::GetShowVoxelOutlines() {
	return showVoxelOutlines;
}

void SliceView::ShowVoxelOutlines(bool show) {
	showVoxelOutlines = show;

	FilterRegions();

	Render();
}

void SliceView::ToggleVoxelOutlines() {
	ShowVoxelOutlines(!showVoxelOutlines);
}

bool SliceView::GetShowRegionOutlines() {
	return showRegionOutlines;
}

void SliceView::ShowRegionOutlines(bool show) {
	showRegionOutlines = show;

	FilterRegions();

	Render();
}

void SliceView::ToggleRegionOutlines() {
	ShowRegionOutlines(!showRegionOutlines);
}

void SliceView::SetFilterRegion(bool filter) {
	filterRegion = filter;

	FilterRegions();
}

void SliceView::ToggleFilterRegion() {
	SetFilterRegion(!filterRegion);
}

void SliceView::UpdatePlane() {
	vtkCamera* cam = renderer->GetActiveCamera();

	plane->SetOrigin(cam->GetFocalPoint());
	plane->SetNormal(cam->GetDirectionOfProjection());

	sliceLocation->UpdateView(cam, plane);
}

void SliceView::Render() {
	renderer->GetRenderWindow()->Render();
}

vtkSmartPointer<vtkRenderer> SliceView::GetRenderer() {
	return renderer;
}

vtkSmartPointer<vtkInteractorStyleSlice> SliceView::GetInteractorStyle() {
	return style;
}

void SliceView::CreateProbe() {
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

void SliceView::UpdateProbe(vtkImageData* data) {
	probe->SetPosition(data->GetCenter());
	probe->SetScale(data->GetSpacing());
	probe->VisibilityOn();
}

void SliceView::CreateInteractionModeLabel() {
	interactionModeLabel = vtkSmartPointer<vtkTextActor>::New();
	interactionModeLabel->SetPosition(10, 10);
	interactionModeLabel->GetTextProperty()->SetFontSize(24);
	interactionModeLabel->GetTextProperty()->SetColor(0.5, 0.5, 0.5);
	interactionModeLabel->VisibilityOff();
	interactionModeLabel->PickableOff();

	renderer->AddActor2D(interactionModeLabel);
}

void SliceView::CreateSlice() {
	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();
	//mapper->JumpToNearestSliceOn();
	mapper->AutoAdjustImageQualityOff();
	mapper->ResampleToScreenPixelsOn();
	//mapper->SetSlabThickness(10);
	//mapper->SetSlabTypeToSum();

	// Image property
	vtkSmartPointer<vtkImageProperty> property = vtkSmartPointer<vtkImageProperty>::New();
	property->SetInterpolationTypeToNearest();

	// Slice
	slice = vtkSmartPointer<vtkImageSlice>::New();
	slice->SetMapper(mapper);
	slice->SetProperty(property);
}

void SliceView::UpdateSlice() {
	// Get image info
	double minValue = data->GetScalarRange()[0];
	double maxValue = data->GetScalarRange()[1];

	slice->GetMapper()->SetInputDataObject(data);
	slice->GetProperty()->SetColorWindow(maxValue - minValue);
	slice->GetProperty()->SetColorLevel(minValue + (maxValue - minValue) / 2);

	renderer->AddActor(slice);
}

void SliceView::CreateLabelSlice() {
	// Mapper
	vtkSmartPointer<vtkImageResliceMapper> mapper = vtkSmartPointer<vtkImageResliceMapper>::New();
	mapper->SliceFacesCameraOn();
	mapper->SliceAtFocalPointOn();
	//mapper->JumpToNearestSliceOn();
	mapper->AutoAdjustImageQualityOff();
	mapper->ResampleToScreenPixelsOn();
	//mapper->SetSlabThickness(10);
	//mapper->SetSlabTypeToMin();

	// Image property
	vtkSmartPointer<vtkImageProperty> property = vtkSmartPointer<vtkImageProperty>::New();
	property->SetInterpolationTypeToNearest();
	property->SetLookupTable(labelColors);
	property->UseLookupTableScalarRangeOn();
	property->SetOpacity(0.1);
	
	// Slice
	labelSlice->SetMapper(mapper);
	labelSlice->SetProperty(property);
}

void SliceView::UpdateLabelSlice() {
	labelSlice->GetMapper()->SetInputDataObject(labels);

	labelSliceRenderer->AddActor(labelSlice);
}

void SliceView::FilterRegions() {
	if (!regions) return;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);
		RegionOutline* outline = region->GetOutline();
		RegionVoxelOutlines* voxelOutlines = region->GetVoxelOutlines();

		if (filterRegion && currentRegion) {			
			bool visible = region == currentRegion;

			outline->GetActor()->SetVisibility(visible && showRegionOutlines);
			voxelOutlines->GetActor()->SetVisibility(visible && showVoxelOutlines);
		}
		else {
			outline->GetActor()->SetVisibility(showRegionOutlines);
			voxelOutlines->GetActor()->SetVisibility(showVoxelOutlines);
		}
	}

	renderer->ResetCameraClippingRange();
	Render();
}

void SliceView::ResetCamera() {

	renderer->GetActiveCamera()->SetFocalPoint(0, 0, 0);
	renderer->GetActiveCamera()->SetPosition(0, 0, -1);
	renderer->GetActiveCamera()->SetViewUp(0, 1, 0);
	renderer->ResetCamera();
	renderer->ResetCameraClippingRange();
}