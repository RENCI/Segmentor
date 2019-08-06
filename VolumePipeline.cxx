#include "VolumePipeline.h"

#include "vtkInteractorStyleVolume.h"

#include <vtkActor.h>
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

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	filterLabels = false;
	smoothSurfaces = false;
	smoothShading = true;
	currentLabel = 0;

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();
	interactor->GetRenderWindow()->AddRenderer(renderer);

	// Interaction
	style = vtkSmartPointer<vtkInteractorStyleVolume>::New();

	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Colors
	labelColors = lut;

	// Probe
	CreateProbe();

	// Create light
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
	filterLabels = false;
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

	renderer->ResetCameraClippingRange();
	Render();
}

void VolumePipeline::SetLabel(unsigned short label) {
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

void VolumePipeline::SetProbeVisiblity(bool visibility) {
	probe->SetVisibility(visibility);
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

void VolumePipeline::SetFilterLabels(bool filter) {
	filterLabels = filter;

	FilterLabels();
}

void VolumePipeline::ToggleFilterLabels() {
	SetFilterLabels(!filterLabels);
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

void VolumePipeline::FilterLabels() {
	for (int i = 0; i < surfaces.size(); i++) {
		RegionSurface* surface = surfaces[i];

		if (filterLabels) surface->GetActor()->SetVisibility(surface->GetRegion()->GetLabel() == currentLabel);
		else surface->GetActor()->VisibilityOn();
	}

	renderer->ResetCameraClippingRange();
	Render();
}