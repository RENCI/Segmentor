#include "SliceView.h"

#include <sstream>

#include "vtkInteractorStyleSlice.h"
#include "vtkImageDataCells.h"

#include <vtkActor.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageProperty.h>
#include <vtkImageReslice.h>
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
#include <vtkTransform.h>

#include "Brush.h"
#include "InteractionEnums.h"
#include "Probe.h"
#include "Region.h"
#include "RegionOutline.h"
#include "RegionSurface.h"
#include "RegionCollection.h"
#include "SegmentorMath.h"
#include "SliceLocation.h"

void SliceView::cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	SliceView* pipeline = static_cast<SliceView*>(clientData);

	pipeline->UpdatePlane();
}

SliceView::SliceView(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	filterMode = FilterNone;
	showRegionOutlines = true;

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
	interactor->GetRenderWindow()->AddRenderer(regionOutlinesRenderer);
	interactor->GetRenderWindow()->AddRenderer(sliceLocationRenderer);
	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Slices
	CreateSlice();
	CreateLabelSlice();

	// Camera callback
	vtkSmartPointer<vtkCallbackCommand> cameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	cameraCallback->SetCallback(cameraChange);
	cameraCallback->SetClientData(this);
	renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cameraCallback);

	// Probe
	probe = new Probe();

	renderer->AddActor(probe->GetActor());

	// Brush
	brush = new Brush();

	regionOutlinesRenderer->AddActor(brush->GetActor());

	// Slice location
	sliceLocation = new SliceLocation(sliceLocationRenderer);

	// Labels
	CreateInteractionModeLabel();
}

SliceView::~SliceView() {
	delete sliceLocation;
	delete brush;
}

void SliceView::Reset() {
	data = nullptr;
	labels = nullptr;

	SetCurrentRegion(nullptr);

	probe->GetActor()->VisibilityOff();
	brush->GetActor()->VisibilityOff();
	sliceLocation->UpdateData(nullptr);
	interactionModeLabel->VisibilityOff();
}

void SliceView::SetImageData(vtkImageData* imageData) {
	// Update slice
	data = imageData;
	UpdateSlice();

	// Update probe
	probe->UpdateData(data);

	// Update brush
	brush->UpdateData(data);

	// Update axes
	sliceLocation->UpdateData(data);

	// Turn on labels
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
		
	//FilterRegions();

	// Make sure slices line up
	vtkCamera* cam = renderer->GetActiveCamera();
	double* f = cam->GetFocalPoint();
	cam->SetFocalPoint(f[0], f[1], (int)f[2]);
}

void SliceView::AddRegion(Region* region) {
	AddRegionActors(region);

	//FilterRegions();
}

void SliceView::AddRegionActors(Region* region) {
	RegionOutline* outline = region->GetOutline();
	outline->SetPlane(plane);
	regionOutlinesRenderer->AddActor(outline->GetActor());

	//renderer->AddActor(region->GetText());
	regionOutlinesRenderer->AddActor(region->GetText());
}

void SliceView::SetCurrentRegion(Region* region) {
	currentRegion = region;

	if (currentRegion) {
		const double* color = region->GetColor();
		probe->GetActor()->GetProperty()->SetColor(color[0], color[1], color[2]);
		brush->GetActor()->GetProperty()->SetColor(color[0], color[1], color[2]);
	}
	else {
		probe->GetActor()->GetProperty()->SetColor(1, 1, 1);
		brush->GetActor()->GetProperty()->SetColor(1, 1, 1);
	}

	//FilterRegions();
}

void SliceView::ShowRegion(Region* region, bool show) {
	region->GetOutline()->GetActor()->SetVisibility(show && showRegionOutlines);
}


void SliceView::UpdateVoxelSize() {
	probe->UpdateData(data);
	brush->UpdateBrush();
	sliceLocation->UpdateData(data);
}

void SliceView::SetShowProbe(bool show) {
	probe->GetActor()->SetVisibility(show);
	brush->GetActor()->SetVisibility(show && brush->GetRadius() > 1);
}

void SliceView::SetProbePosition(double x, double y, double z) {
	vtkCamera* cam = renderer->GetActiveCamera();

	double p1[3] = { x , y, z };
	double p2[3];
	vtkPlane::ProjectPoint(p1, cam->GetFocalPoint(), cam->GetDirectionOfProjection(), p2);

	double* spacing = data->GetSpacing();
	double s = spacing[0];
	
	if (sqrt(vtkMath::Distance2BetweenPoints(p1, p2)) < s) {
		probe->GetActor()->GetProperty()->SetRepresentationToWireframe();
		brush->GetActor()->GetProperty()->SetRepresentationToWireframe();
	}
	else {
		probe->GetActor()->GetProperty()->SetRepresentationToPoints();
		brush->GetActor()->GetProperty()->SetRepresentationToPoints();
	}	

	probe->GetActor()->SetPosition(p2);
	brush->GetActor()->SetPosition(p2);
}

void SliceView::SetInteractionMode(enum InteractionMode mode) {
	std::string s = mode == NavigationMode ? "Navigation mode" : "Edit mode";

	interactionModeLabel->SetInput(s.c_str());

	style->SetMode(mode);
}

void SliceView::SetFilterMode(enum FilterMode mode) {
	//filterMode = mode;

	//FilterRegions();
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

bool SliceView::GetShowRegionOutlines() {
	return showRegionOutlines;
}

void SliceView::ShowRegionOutlines(bool show) {
	showRegionOutlines = show;

	//FilterRegions();
}

void SliceView::ToggleRegionOutlines() {
	ShowRegionOutlines(!showRegionOutlines);
}

double SliceView::GetWindow() {
	return slice->GetProperty()->GetColorWindow();
}

double SliceView::GetLevel() {
	return slice->GetProperty()->GetColorLevel();
}

void SliceView::SetWindow(double window) {
	slice->GetProperty()->SetColorWindow(window);

	Render();
}

void SliceView::SetLevel(double level) {
	slice->GetProperty()->SetColorLevel(level);

	Render();
}

void SliceView::RescaleFull() {
	vtkSmartPointer<vtkImageData> scaleSlice = GetSlice();

	double minValue = scaleSlice->GetScalarRange()[0];
	double maxValue = scaleSlice->GetScalarRange()[1];

	double range = maxValue - minValue;

	slice->GetProperty()->SetColorWindow(range);
	slice->GetProperty()->SetColorLevel(minValue + range / 2);

	Render();
}

void SliceView::RescalePartial() {
	vtkSmartPointer<vtkImageData> scaleSlice = GetSlice();

	SegmentorMath::OtsuValues otsu = SegmentorMath::OtsuThreshold(scaleSlice);

	double minValue = otsu.backgroundMean;
	double maxValue = otsu.foregroundMean;

	double range = maxValue - minValue;;

	slice->GetProperty()->SetColorWindow(range * 1.5);
	slice->GetProperty()->SetColorLevel(minValue + range / 2);

	Render();
}

vtkSmartPointer<vtkImageData> SliceView::GetSlice() {
	vtkCamera* camera = renderer->GetActiveCamera();
	double* wxyz = camera->GetOrientationWXYZ();

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->Translate(camera->GetFocalPoint());
	transform->RotateWXYZ(wxyz[0], wxyz[1], wxyz[2], wxyz[3]);

	vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
	reslice->SetOutputDimensionality(2);
	reslice->SetResliceAxes(transform->GetMatrix());
	reslice->SetInputDataObject(data);
	reslice->Update();
	
	return reslice->GetOutput();
}

void SliceView::SetOverlayOpacity(double opacity) {
	labelSlice->GetProperty()->SetOpacity(opacity);

	Render();
}

void SliceView::UpdatePlane() {
	vtkCamera* cam = renderer->GetActiveCamera();

	plane->SetOrigin(cam->GetFocalPoint());
	plane->SetNormal(cam->GetDirectionOfProjection());

	sliceLocation->UpdateView(cam, plane);
}

void SliceView::SetBrushRadius(int radius) {
	brush->SetRadius(radius);
	brush->GetActor()->SetVisibility(radius > 1);
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

		// XXX: Setting based on surface visiblity probably indicates that the logic should be pushed up to visualization container,
		// with a method in region for turning on and off individual pieces

		bool visible = region->GetSurface()->GetActor()->GetVisibility();

		outline->GetActor()->SetVisibility(visible && showRegionOutlines);
	}

	renderer->ResetCameraClippingRange();
	Render();
}

void SliceView::ResetCamera() {
	renderer->GetActiveCamera()->SetFocalPoint(0, 0, 0);
	renderer->GetActiveCamera()->SetPosition(0, 0, 1);
	renderer->GetActiveCamera()->SetViewUp(0, 1, 0);
	renderer->ResetCamera();
	renderer->ResetCameraClippingRange();
}