#include "VolumeView.h"

#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkImageData.h>
#include <vtkInteractorStyle.h>
#include <vtkLight.h>
#include <vtkOutlineCornerFilter.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include "vtkInteractorStyleVolume.h"

#include "InteractionEnums.h"
#include "Region.h"
#include "RegionSurface.h"
#include "RegionHighlight3D.h"
#include "RegionCollection.h"

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

void VolumeView::cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	VolumeView* pipeline = static_cast<VolumeView*>(clientData);

	pipeline->UpdatePlane();
}

VolumeView::VolumeView(vtkRenderWindowInteractor* interactor) {
	filterMode = FilterNone;
	smoothSurfaces = false;
	smoothShading = true;
	
	regions = nullptr;
	currentRegion = nullptr;
	highlightRegion = nullptr;

	neighborOpacity = 0.25;

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
	CreateProbe();

	// Plane
	CreatePlane();

	// Corners
	CreateCorners();

	// Interaction mode label
	CreateInteractionModeLabel();

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
	SetCurrentRegion(nullptr);
	HighlightRegion(nullptr);

	probe->VisibilityOff();
	plane->VisibilityOff();
	corners->VisibilityOff();
	interactionModeLabel->VisibilityOff();
}

void VolumeView::SetRegions(vtkImageData* data, RegionCollection* newRegions) {
	regions = newRegions;

	// Reset
	filterMode = FilterNone;
	currentRegion = nullptr;
	highlightRegion = nullptr;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		AddRegion(regions->Get(it));
	}

	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();

	// Update plane
	UpdatePlane(data);

	// Update corners
	UpdateCorners(data);
	corners->VisibilityOn();

	// Turn on interaction mode
	interactionModeLabel->VisibilityOn();

	renderer->ResetCameraClippingRange();
}

void VolumeView::AddRegion(Region* region) {
	RegionSurface* surface = region->GetSurface();
	RegionHighlight3D* highlight = region->GetHighlight3D();

	surface->SetSmoothSurface(smoothSurfaces);
	surface->SetSmoothShading(smoothShading);

	highlight->GetActor()->VisibilityOff();
	highlight->SetCamera(renderer->GetActiveCamera());

	renderer->AddActor(surface->GetActor());
	renderer->AddActor(highlight->GetActor());
}

void VolumeView::SetCurrentRegion(Region* region) {
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

	FilterRegions();
}

void VolumeView::SetShowProbe(bool show) {
	probe->SetVisibility(show && regions != nullptr);
}

void VolumeView::SetProbePosition(double x, double y, double z) {
	probe->SetPosition(x, y, z);
}

void VolumeView::SetInteractionMode(enum InteractionMode mode) {
	std::string s = mode == NavigationMode ? "Navigation mode" : "Edit mode";
	interactionModeLabel->SetInput(s.c_str());

	style->SetMode(mode);
}

void VolumeView::SetFilterMode(enum FilterMode mode) {
	filterMode = mode;

	FilterRegions();
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

void VolumeView::SetNeighborOpacity(double opacity) {
	neighborOpacity = opacity;

	FilterRegions();
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

void VolumeView::CreateProbe() {
	vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(source->GetOutputPort());

	probe = vtkSmartPointer<vtkActor>::New();
	probe->SetMapper(mapper);
	probe->GetProperty()->SetRepresentationToWireframe();
	probe->GetProperty()->LightingOff();
	probe->GetProperty()->SetLineWidth(2);
	probe->VisibilityOff();
	probe->PickableOff();

	renderer->AddActor(probe);
}

void VolumeView::UpdateProbe(vtkImageData* data) {
	probe->SetPosition(data->GetCenter());
	probe->SetScale(data->GetSpacing());
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

void VolumeView::FilterRegions() {
	if (!regions) return;

	vtkCamera * cam = renderer->GetActiveCamera();

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);
		RegionSurface* surface = region->GetSurface();

		// Reset opacity
		surface->GetActor()->GetProperty()->SetOpacity(1);

		// Always show current region
		if (currentRegion && region == currentRegion) {
			surface->GetActor()->VisibilityOn();
			continue;
		}

		switch (filterMode) {
		case FilterNone:
			surface->GetActor()->VisibilityOn();
			break;

		case FilterPlane:
			surface->GetActor()->SetVisibility(surface->IntersectsPlane(cam->GetFocalPoint(), cam->GetDirectionOfProjection()));
			break;

		case FilterNeighbors:
			if (currentRegion) {
				double d = sqrt(vtkMath::Distance2BetweenPoints(currentRegion->GetCenter(), region->GetCenter()));
				double r = (currentRegion->GetLength() + region->GetLength()) / 4;

				if (d <= r * 1.05) {
					surface->GetActor()->VisibilityOn();
					surface->GetActor()->GetProperty()->SetOpacity(neighborOpacity);
				}
				else {
					surface->GetActor()->VisibilityOff();
				}
			}
			else {
				surface->GetActor()->VisibilityOn();
			}
			break;

		case FilterRegion: 
			if (currentRegion) {
				surface->GetActor()->VisibilityOff();
			}
			else {
				surface->GetActor()->VisibilityOn();
			}
			break;
		}
	}

	if (highlightRegion) {
		highlightRegion->GetSurface()->GetActor()->VisibilityOn();
	}

	renderer->ResetCameraClippingRange();
	Render();
}