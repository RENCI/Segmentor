#include "VolumePipeline.h"

#include "vtkInteractorStyleVolume.h"

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkImageData.h>
#include <vtkInteractorStyle.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include "Region.h"
#include "RegionSurface.h"

#include <vtkCallbackCommand.h>
#include <vtkPlaneSource.h>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	VolumePipeline* pipeline = static_cast<VolumePipeline*>(clientData);

	pipeline->UpdatePlane();
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	filterLabel = false;
	filterPlane = false;
	smoothSurfaces = false;
	smoothShading = true;
	currentLabel = 0;

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

	// Colors
	labelColors = lut;

	// Probe
	CreateProbe();

	// Plane
	CreatePlane();

	// Lighting
	double lightPosition[3] = { 0, 0.5, 1 };	
	double lightFocalPoint[3] = { 0, 0, 0 };

	vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	light->SetLightTypeToCameraLight();
	light->SetPosition(lightPosition);
	light->SetFocalPoint(lightFocalPoint);

	renderer->AddLight(light);
}

VolumePipeline::~VolumePipeline() {
	RemoveSurfaces();
}

void VolumePipeline::SetRegions(vtkImageData* data, std::vector<Region*> regions) {
	// Reset
	filterLabel = false;
	currentLabel = 0;
	
	RemoveSurfaces();

	for (int i = 0; i < regions.size(); i++) {
		RegionSurface* surface = new RegionSurface(regions[i], labelColors);
		surface->SetSmoothSurfaces(smoothSurfaces);
		surface->SetSmoothShading(smoothShading);

		renderer->AddActor(surface->GetActor());

		surfaces.push_back(surface);
	}

	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();

	// Update plane
	UpdatePlane(data);

	renderer->ResetCameraClippingRange();
	Render();
}

void VolumePipeline::SetCurrentLabel(unsigned short label) {
	currentLabel = label;

	if (currentLabel > 0) {
		double color[3];
		labelColors->GetColor(currentLabel, color);
		probe->GetProperty()->SetColor(color);
	}
	else {
		probe->GetProperty()->SetColor(1, 1, 1);
	}

	FilterLabels();
}

void VolumePipeline::SetShowProbe(bool show) {
	probe->SetVisibility(show);
}

void VolumePipeline::SetProbePosition(double x, double y, double z) {
	probe->SetPosition(x, y, z);
}

void VolumePipeline::SetSmoothSurfaces(bool smooth) {
	smoothSurfaces = smooth;

	for (int i = 0; i < surfaces.size(); i++) {
		surfaces[i]->SetSmoothSurfaces(smoothSurfaces);
	}

	Render();
}

void VolumePipeline::ToggleSmoothSurfaces() {
	SetSmoothSurfaces(!smoothSurfaces);
}

void VolumePipeline::SetSmoothShading(bool smooth) {
	smoothShading = smooth;

	for (int i = 0; i < surfaces.size(); i++) {
		surfaces[i]->SetSmoothShading(smoothShading);
	}

	Render();
}

void VolumePipeline::ToggleSmoothShading() {
	SetSmoothShading(!smoothShading);
}

void VolumePipeline::SetFilterLabel(bool filter) {
	filterLabel = filter;

	FilterLabels();
}

void VolumePipeline::ToggleFilterLabel() {
	SetFilterLabel(!filterLabel);
}

void VolumePipeline::SetFilterPlane(bool filter) {
	filterPlane = filter;

	FilterLabels();
}

void VolumePipeline::ToggleFilterPlane() {
	SetFilterPlane(!filterPlane);
}

void VolumePipeline::UpdatePlane() {
	vtkCamera* cam = renderer->GetActiveCamera();

	planeSource->SetCenter(cam->GetFocalPoint());
	planeSource->SetNormal(cam->GetDirectionOfProjection());
}

void VolumePipeline::SetShowPlane(bool show) {
	plane->SetVisibility(show);
	planeWire->SetVisibility(show);
	Render();
}

void VolumePipeline::ToggleShowPlane() {
	SetShowPlane(!plane->GetVisibility());
}

void VolumePipeline::Render() {
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* VolumePipeline::GetRenderer() {
	return renderer;
}

vtkInteractorStyleVolume* VolumePipeline::GetInteractorStyle() {
	return style;
}

void VolumePipeline::RemoveSurfaces() {
	for (int i = 0; i < surfaces.size(); i++) {
		delete surfaces[i];
	}
}

void VolumePipeline::CreateProbe() {
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

void VolumePipeline::UpdateProbe(vtkImageData* data) {
	probe->SetPosition(data->GetCenter());
	probe->SetScale(data->GetSpacing());
}

void VolumePipeline::CreatePlane() {
	planeSource = vtkSmartPointer<vtkPlaneSource>::New();
	planeSource->SetXResolution(50);
	planeSource->SetYResolution(50);

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

	planeWire = vtkSmartPointer<vtkActor>::New();
	planeWire->SetMapper(mapperWire);
	planeWire->GetProperty()->SetRepresentationToWireframe();
	planeWire->GetProperty()->LightingOff();
	planeWire->GetProperty()->SetColor(0, 0, 0);
	planeWire->GetProperty()->SetOpacity(0.1);
	planeWire->VisibilityOff();
	planeWire->PickableOff();

	renderer->AddActor(plane);
	renderer->AddActor(planeWire);
}

void VolumePipeline::UpdatePlane(vtkImageData* data) {	
	double bb[6];
	data->GetBounds(bb);

	double s = bb[1] - bb[0];

	planeSource->SetOrigin(0, 0, 0);
	planeSource->SetPoint1(s, 0, 0);
	planeSource->SetPoint2(0, s, 0);

	UpdatePlane();
}

void VolumePipeline::FilterLabels() {
	for (int i = 0; i < surfaces.size(); i++) {
		RegionSurface* surface = surfaces[i];

		if (filterPlane) {
			vtkCamera* cam = renderer->GetActiveCamera();
			bool ix = surface->IntersectsPlane(cam->GetFocalPoint(), cam->GetDirectionOfProjection());

			//if (ix) surface->GetActor()->GetProperty()->SetRepresentationToSurface();
			//else surface->GetActor()->GetProperty()->SetRepresentationToPoints();
			
			//surface->GetActor()->GetProperty()->SetOpacity(ix ? 1 : 0.1);
			surface->GetActor()->SetVisibility(ix);
		}
		else {
			//surface->GetActor()->GetProperty()->SetRepresentationToSurface();
			//surface->GetActor()->GetProperty()->SetOpacity(1.0);
			surface->GetActor()->VisibilityOn();
		}
			
		if (filterLabel) {
			surface->GetActor()->SetVisibility(surface->GetRegion()->GetLabel() == currentLabel);
		}
		else if (!filterPlane) {
			surface->GetActor()->VisibilityOn();
		}
	}

	renderer->ResetCameraClippingRange();
	Render();
}