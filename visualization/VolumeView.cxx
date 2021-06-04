#include "VolumeView.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkCubeAxesActor.h>
#include <vtkCubeSource.h>
#include <vtkExtractVOI.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageMask.h>
#include <vtkInteractorStyle.h>
#include <vtkLight.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkPassThrough.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include "vtkInteractorStyleVolume.h"

#include "Brush.h"
#include "InteractionEnums.h"
#include "Probe.h"
#include "Region.h"
#include "RegionSurface.h"
#include "RegionHighlight3D.h"
#include "RegionCollection.h"
#include "SegmentorMath.h"

#include <vector>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

void VolumeView::cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	VolumeView* pipeline = static_cast<VolumeView*>(clientData);

	pipeline->UpdatePlane();
}

VolumeView::VolumeView(vtkRenderWindowInteractor* interactor) {
	data = nullptr;
	labels = nullptr;

	smoothSurfaces = false;
	smoothShading = false;
	volumeRendering = false;
	
	regions = nullptr;
	currentRegion = nullptr;
	highlightRegion = nullptr;

	visibleOpacity = 1.0;

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();

	style = vtkSmartPointer<vtkInteractorStyleVolume>::New();

	interactor->GetRenderWindow()->AddRenderer(renderer);
	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Camera callback
	vtkSmartPointer <vtkCallbackCommand> cameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	cameraCallback->SetCallback(cameraChange);
	cameraCallback->SetClientData(this);
	renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cameraCallback);

	// Probe
	probe = new Probe(1.1, true);

	renderer->AddActor(probe->GetActor());

	// Brush
	brush = new Brush();

	renderer->AddActor(brush->GetActor());

	// Plane
	CreatePlane();

	// Corners
	CreateCorners();

	// Axes
	CreateAxes();

	// Interaction mode label
	CreateInteractionModeLabel();

	// Volume rendering
	CreateVolumeRenderer();

	// Lighting
	double lightPosition[3] = { 0, 0.5, 1 };	
	double lightFocalPoint[3] = { 0, 0, 0 };

	vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	light->SetLightTypeToCameraLight();
	light->SetPosition(lightPosition);
	light->SetFocalPoint(lightFocalPoint);

	renderer->AddLight(light);
}

VolumeView::~VolumeView() {
}

void VolumeView::Reset() {
	data = nullptr;
	labels = nullptr;

	SetCurrentRegion(nullptr);
	HighlightRegion(nullptr);

	probe->GetActor()->VisibilityOff();
	brush->GetActor()->VisibilityOff();
	plane->VisibilityOff();
	corners->VisibilityOff();
	interactionModeLabel->VisibilityOff();

	volume->VisibilityOff();
}

void VolumeView::SetImageData(vtkImageData* imageData) {
	data = imageData;

	brush->UpdateData(data);

	UpdateVolumeRenderer();
}

void VolumeView::Enable(bool enable) {
	vtkRenderWindow* window = style->GetInteractor()->GetRenderWindow();

	if (enable) {
		// Remove dummy renderer
		vtkRendererCollection* renderers = window->GetRenderers();
		renderers->InitTraversal();
		for (vtkRenderer* ren = renderers->GetFirstRenderer(); ren != nullptr; ren = renderers->GetNextItem()) {
			if (ren != renderer) {
				window->RemoveRenderer(ren);
			}
		}
	}
	else {		
		// Hide with dummy renderer
		vtkSmartPointer<vtkRenderer> dummy = vtkSmartPointer<vtkRenderer>::New();
		dummy->InteractiveOff();
		window->AddRenderer(dummy);
	}

	Render();
}

void VolumeView::UpdateVoxelSize(vtkImageData* data) {
	probe->UpdateData(data);
	brush->UpdateBrush();
	UpdatePlane(data);
	UpdateAxes(data);
}

void VolumeView::SetRegions(vtkImageData* imageLabels, RegionCollection* newRegions) {
	labels = imageLabels;
	regions = newRegions;

	// Reset
	currentRegion = nullptr;
	highlightRegion = nullptr;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		AddRegion(regions->Get(it));
	}

	// Update probe
	probe->UpdateData(data);

	// Update brush
	//brush->UpdateData(data);

	// Update plane
	UpdatePlane(data);

	// Update corners
	UpdateCorners(data);
	corners->VisibilityOn();

	// Update axes
	UpdateAxes(data);
	axes->VisibilityOn();

	// Turn on interaction mode
	interactionModeLabel->VisibilityOn();

	renderer->ResetCameraClippingRange();
}

void VolumeView::AddRegion(Region* region) {
	RegionSurface* surface = region->GetSurface();
	RegionHighlight3D* highlight = region->GetHighlight3D();

	surface->SetSmoothSurface(smoothSurfaces);
	surface->SetSmoothShading(smoothShading);
	surface->SetRenderMode(volumeRendering ? RegionSurface::CullFrontFace : RegionSurface::Normal);

	highlight->GetActor()->VisibilityOff();
	highlight->SetCamera(renderer->GetActiveCamera());

	renderer->AddActor(surface->GetActor());
	renderer->AddActor(highlight->GetActor());

#ifdef SHOW_REGION_BOX
	renderer->AddActor(region->GetBox());
#endif
}

void VolumeView::SetCurrentRegion(Region* region) {
	currentRegion = region;

	if (currentRegion) {
		const double* color = region->GetColor();
		probe->GetActor()->GetProperty()->SetColor(color[0], color[1], color[2]);
		brush->GetActor()->GetProperty()->SetColor(color[0], color[1], color[2]);

		const int* voi = currentRegion->GetExtent();
	}
	else {
		probe->GetActor()->GetProperty()->SetColor(1, 1, 1);
		brush->GetActor()->GetProperty()->SetColor(1, 1, 1);
	}
}

void VolumeView::HighlightRegion(Region* region) {
	if (!regions) return;

	highlightRegion = region;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		regions->Get(it)->GetHighlight3D()->GetActor()->VisibilityOff();
	}

	if (highlightRegion) {
		highlightRegion->GetHighlight3D()->Update();
		highlightRegion->GetHighlight3D()->GetActor()->VisibilityOn();
	}

	renderer->ResetCameraClippingRange();
}

void VolumeView::ShowRegion(Region* region, bool show) {
	region->GetSurface()->GetActor()->SetVisibility(show);
}

void VolumeView::SetShowProbe(bool show) {
	show = show && regions != nullptr;

	probe->GetActor()->SetVisibility(show);
	brush->GetActor()->SetVisibility(show && brush->GetRadius() > 1);
}

void VolumeView::SetProbePosition(double x, double y, double z) {
	probe->GetActor()->SetPosition(x, y, z);
	brush->GetActor()->SetPosition(x, y, z);
}

void VolumeView::SetInteractionMode(enum InteractionMode mode) {
	std::string s =
		mode == NavigationMode ? "Navigation mode" :
		mode == EditMode ? "Edit mode" :
		mode == AddMode ? "" :
		mode == MergeMode ? "Merge region" :
		mode == GrowMode ? "" :
		mode == VisibleMode ? "Toggle region visibility" :
		"";

	interactionModeLabel->SetInput(s.c_str());

	style->SetMode(mode);
}

bool VolumeView::GetSmoothSurfaces() {
	return smoothSurfaces;
}

void VolumeView::SetSmoothSurfaces(bool smooth) {
	smoothSurfaces = smooth;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		regions->Get(it)->GetSurface()->SetSmoothSurface(smoothSurfaces);
	}

	Render();
}

void VolumeView::ToggleSmoothSurfaces() {
	SetSmoothSurfaces(!smoothSurfaces);
}

bool VolumeView::GetSmoothShading() {
	return smoothShading;
}

void VolumeView::SetSmoothShading(bool smooth) {
	smoothShading = smooth;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		regions->Get(it)->GetSurface()->SetSmoothShading(smoothShading);
	}

	Render();
}

void VolumeView::ToggleSmoothShading() {
	SetSmoothShading(!smoothShading);
}

bool VolumeView::GetVolumeRendering() {
	return volumeRendering;
}

void VolumeView::SetVolumeRendering(bool useVolumeRendering) {
	volumeRendering = useVolumeRendering;

	volume->SetVisibility(volumeRendering);
	
	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		regions->Get(it)->GetSurface()->SetRenderMode(volumeRendering ? RegionSurface::CullFrontFace : RegionSurface::Normal);
	}

	Render();
}

void VolumeView::ToggleVolumeRendering() {
	SetVolumeRendering(!volumeRendering);
}

void VolumeView::UpdatePlane() {
	vtkCamera* cam = renderer->GetActiveCamera();

	planeSource->SetCenter(cam->GetFocalPoint());
	planeSource->SetNormal(cam->GetDirectionOfProjection());
}

bool VolumeView::GetShowPlane() {
	return plane->GetVisibility();
}

void VolumeView::SetShowPlane(bool show) {
	plane->SetVisibility(show);
	planeWire->SetVisibility(show);
	Render();
}

void VolumeView::ToggleShowPlane() {
	SetShowPlane(!plane->GetVisibility());
}

double VolumeView::GetVisibleOpacity() {
	return visibleOpacity;
}

void VolumeView::SetVisibleOpacity(double opacity, bool apply) {
	visibleOpacity = opacity;

	UpdateVisibleOpacity(apply);

	Render();
}

void VolumeView::UpdateVisibleOpacity(bool apply) {
	if (!regions) return;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);
		RegionSurface* surface = region->GetSurface();

		double o = !apply || region == currentRegion ? 1 : visibleOpacity;

		surface->GetActor()->GetProperty()->SetOpacity(o);
	}

	UpdateVolumeMask(apply);
}

void VolumeView::SetBrushRadius(int radius) {
	brush->SetRadius(radius);
	brush->GetActor()->SetVisibility(radius > 1);
}

void VolumeView::SetWindowLevel(double window, double level) {
	UpdateVolumeRenderingTransferFunctions(level - window * 0.5, level + window * 0.5);
}

void VolumeView::UpdateVolumeMask(bool filter) {
	if (regions && filter) {
		vtkSmartPointer<vtkImageData> mask = volumeCopy->GetOutput();

		const int* extent = mask->GetExtent();

		for (int i = extent[0]; i <= extent[1]; i++) {
			for (int j = extent[2]; j <= extent[3]; j++) {
				for (int k = extent[4]; k <= extent[5]; k++) {
					unsigned char* p = static_cast<unsigned char*>(mask->GetScalarPointer(i, j, k));
					*p = 0;
				}
			}
		}

		for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
			Region* region = regions->Get(it);
			const int* extent = region->GetExtent();
			
			if (region == currentRegion || region->GetVisible()) {
				for (int i = extent[0]; i <= extent[1]; i++) {
					for (int j = extent[2]; j <= extent[3]; j++) {
						for (int k = extent[4]; k <= extent[5]; k++) {
							// Current label
							unsigned char* p = static_cast<unsigned char*>(mask->GetScalarPointer(i, j, k));
							*p = 1;
						}
					}
				}
			}
		}

		mask->Modified();

		volumeMask->SetMaskInputData(mask);
		
		volumeMapper->SetInputConnection(volumeMask->GetOutputPort());
	}
	else {
		volumeMapper->SetInputDataObject(data);
	}
}

void VolumeView::UpdateVolumeRenderingTransferFunctions(double x1, double x2) {
	// Opacity
	volumeOpacity->RemoveAllPoints();
	volumeOpacity->AddPoint(x1, 0.0);
	volumeOpacity->AddPoint(x2, 0.5);

	// Colors
	// Paraview diverging
	const int numColors = 3;
	double colors[numColors][3] = {
		{ 59, 76, 192 },
		{ 221, 221, 221 },
		{ 180, 4, 38 }
	};

	for (int i = 0; i < numColors; i++) {
		for (int j = 0; j < 3; j++) {
			colors[i][j] /= 255.0;
		}
	}

	volumeColor->RemoveAllPoints();
	for (int i = 0; i < numColors; i++) {
		double x = x1 + (double)i / (numColors - 1) * (x2 - x1);
		volumeColor->AddRGBPoint(x, colors[i][0], colors[i][1], colors[i][2]);
	}
}

void VolumeView::Render() {
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* VolumeView::GetRenderer() {
	return renderer;
}

vtkInteractorStyleVolume* VolumeView::GetInteractorStyle() {
	return style;
}

void VolumeView::CreateInteractionModeLabel() {
	interactionModeLabel = vtkSmartPointer<vtkTextActor>::New();
	interactionModeLabel->SetPosition(10, 10);
	interactionModeLabel->GetTextProperty()->SetFontSize(24);
	interactionModeLabel->GetTextProperty()->SetColor(0.5, 0.5, 0.5);
	interactionModeLabel->VisibilityOff();
	interactionModeLabel->PickableOff();

	renderer->AddActor2D(interactionModeLabel);
}

void VolumeView::CreateVolumeRenderer() {
	volumeCopy = vtkSmartPointer<vtkImageCast>::New();
	volumeCopy->SetOutputScalarTypeToUnsignedChar();

	volumeMask = vtkSmartPointer<vtkImageMask>::New();
	volumeMask->SetMaskInputData(volumeCopy->GetOutput());

	volumeMapper = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->AutoAdjustSampleDistancesOn();
	volumeMapper->SetSampleDistance(0.1);
	volumeMapper->SetInteractiveSampleDistance(0.1);
	volumeMapper->SetImageSampleDistance(0.5);
	volumeMapper->SetMaximumImageSampleDistance(2);
	
	volumeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();

	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();

	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->SetScalarOpacity(volumeOpacity);
	volumeProperty->SetColor(volumeColor);

	volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);
	volume->VisibilityOff();
	volume->PickableOff();

	renderer->AddVolume(volume);
}

void VolumeView::UpdateVolumeRenderer() {		
	if (!data) return;

	// Start with full volume
	volumeCopy->SetInputDataObject(data);
	volumeCopy->Update();

	volumeMask->SetInputDataObject(data);

	volumeMapper->SetInputDataObject(data);
	volume->SetVisibility(volumeRendering);	
		
	// Initialize window level
	SegmentorMath::OtsuValues otsu = SegmentorMath::OtsuThreshold(data);

	double minValue = otsu.backgroundMean;
	double maxValue = otsu.foregroundMean;

	double range = maxValue - minValue;;

	double window = range;
	double level = minValue + range / 2;

	SetWindowLevel(window, level);
	style->SetWindowLevel(window, level);
}

void VolumeView::CreatePlane() {
	planeSource = vtkSmartPointer<vtkPlaneSource>::New();
	planeSource->SetXResolution(100);
	planeSource->SetYResolution(100);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(planeSource->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapperWire = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapperWire->SetInputConnection(planeSource->GetOutputPort());

	plane = vtkSmartPointer<vtkActor>::New();
	plane->SetMapper(mapper);
	plane->GetProperty()->LightingOff();
	plane->GetProperty()->SetOpacity(0.2);
	plane->VisibilityOff();
	plane->PickableOff();
	plane->UseBoundsOff();

	planeWire = vtkSmartPointer<vtkActor>::New();
	planeWire->SetMapper(mapperWire);
	planeWire->GetProperty()->SetRepresentationToWireframe();
	planeWire->GetProperty()->LightingOff();
	planeWire->GetProperty()->SetColor(0, 0, 0);
	planeWire->GetProperty()->SetOpacity(0.1);
	planeWire->VisibilityOff();
	planeWire->PickableOff();
	planeWire->UseBoundsOff();

	renderer->AddActor(plane);
	renderer->AddActor(planeWire);
}

void VolumeView::UpdatePlane(vtkImageData* data) {	
	double* bounds = data->GetBounds();
	double length = data->GetLength();

	double s = length * 2;

	planeSource->SetOrigin(0, 0, 0);
	planeSource->SetPoint1(s, 0, 0);
	planeSource->SetPoint2(0, s, 0);

	// Clip plane
	double p1[3] = { bounds[0], bounds[2], bounds[4] };
	double p2[3] = { bounds[1], bounds[3], bounds[5] };
	double n[6][3] = {
		{ 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { -1, 0, 0 }, { 0, -1, 0 }, { 0, 0, -1 }
	};

	plane->GetMapper()->RemoveAllClippingPlanes();
	planeWire->GetMapper()->RemoveAllClippingPlanes();
	for (int i = 0; i < 6; i++) {
		vtkSmartPointer<vtkPlane> clip = vtkSmartPointer<vtkPlane>::New();
		clip->SetOrigin(i < 3 ? p1 : p2);
		clip->SetNormal(n[i]);

		plane->GetMapper()->AddClippingPlane(clip);
		planeWire->GetMapper()->AddClippingPlane(clip);
	}

	UpdatePlane();
}

void VolumeView::CreateCorners() {
	cornerFilter = vtkSmartPointer<vtkOutlineCornerFilter>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(cornerFilter->GetOutputPort());

	corners = vtkSmartPointer<vtkActor>::New();
	corners->GetProperty()->SetColor(0.5, 0.5, 0.5);
	corners->GetProperty()->LightingOff();
	corners->SetMapper(mapper);
	corners->PickableOff();
	corners->VisibilityOff();

	renderer->AddActor(corners);
}

void VolumeView::UpdateCorners(vtkImageData* data) {
	cornerFilter->SetInputDataObject(data);
}

void VolumeView::CreateAxes() {
	double color[3] = { 0.5, 0.5, 0.5 };

	axes = vtkSmartPointer<vtkCubeAxesActor>::New();
	axes->SetCamera(renderer->GetActiveCamera());
	axes->GetXAxesLinesProperty()->SetColor(color);
	axes->GetYAxesLinesProperty()->SetColor(color);
	axes->GetZAxesLinesProperty()->SetColor(color);
	axes->GetTitleTextProperty(0)->SetColor(color);
	axes->GetTitleTextProperty(1)->SetColor(color);
	axes->GetTitleTextProperty(2)->SetColor(color);
	axes->GetLabelTextProperty(0)->SetColor(color);
	axes->GetLabelTextProperty(1)->SetColor(color);
	axes->GetLabelTextProperty(2)->SetColor(color);
	axes->GetProperty()->LightingOff();
	axes->PickableOff();
	axes->VisibilityOff();

	renderer->AddActor(axes);
}

void VolumeView::UpdateAxes(vtkImageData* data) {
	axes->SetBounds(data->GetBounds());
	axes->SetFlyModeToStaticTriad();
}