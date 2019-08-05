#include "VolumePipeline.h"

#include "vtkInteractorStyleVolume.h"

#include <vtkActor.h>
#include <vtkCubeSource.h>
#include <vtkExtractVOI.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>
#include <vtkInteractorStyle.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include <algorithm>

#include "Region.h"

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut) {
	thresholdLabels = false;
	smoothSurfaces = true;
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
}

void VolumePipeline::SetRegions(vtkImageData* data, std::vector<Region*> regions) {
	// Reset
	thresholdLabels = false;
	smoothSurfaces = true;
	currentLabel = 0;
	
	regionActors.clear();

	for (int i = 0; i < regions.size(); i++) {
		Region* region = regions[i];

		//		vtkSmartPointer<vtkContourFilter> contour = vtkSmartPointer<vtkContourFilter>::New();
		//		vtkSmartPointer<vtkFlyingEdges3D> contour = vtkSmartPointer<vtkFlyingEdges3D>::New();
		vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New();
		contour->SetValue(0, region->GetLabel());
		contour->ComputeNormalsOff();
		contour->ComputeGradientsOff();
		contour->SetInputConnection(region->GetOutput());
		//		contour->SetInputDataObject(data);

		// Smoother
		int smoothingIterations = 8;
		double passBand = 0.01;
		double featureAngle = 120.0;

		vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
		smoother->SetNumberOfIterations(smoothingIterations);
		//smoother->BoundarySmoothingOff();
		//smoother->FeatureEdgeSmoothingOff();
		//smoother->SetFeatureAngle(featureAngle);
		smoother->SetPassBand(passBand);
		//smoother->NonManifoldSmoothingOn();
		smoother->NormalizeCoordinatesOn();
		smoother->SetInputConnection(contour->GetOutputPort());

		vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
		normals->ComputePointNormalsOn();
		normals->SplittingOff();
		normals->SetInputConnection(contour->GetOutputPort());
		//		normals->SetInputConnection(smoother->GetOutputPort());

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
				mapper->SetInputConnection(normals->GetOutputPort());
		//		mapper->SetInputConnection(smoother->GetOutputPort());
		//mapper->SetInputConnection(contour->GetOutputPort());
		mapper->ScalarVisibilityOff();

		double color[3];
		labelColors->GetColor(region->GetLabel(), color);

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(color);
		actor->GetProperty()->SetDiffuse(1.0);
		actor->GetProperty()->SetAmbient(0.1);
		actor->GetProperty()->SetSpecular(0.0);

		regionActors.push_back(actor);

		renderer->AddActor(actor);
	}

	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();

	renderer->ResetCameraClippingRange();
	Render();
}

void VolumePipeline::SetProbeVisiblity(bool visibility) {
	probe->SetVisibility(visibility);
}

void VolumePipeline::SetProbePosition(double x, double y, double z) {
	probe->SetPosition(x, y, z);
}

void VolumePipeline::SetSmoothSurfaces(bool smooth) {
	smoothSurfaces = smooth;

	Render();
}

void VolumePipeline::ToggleSmoothSurfaces() {
	SetSmoothSurfaces(!smoothSurfaces);
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
}

void VolumePipeline::SetThresholdLabels(bool doThreshold) {
	thresholdLabels = doThreshold;

	for (int i = 0; i < regionActors.size(); i++) {
		if (thresholdLabels) regionActors[i]->SetVisibility(i == currentLabel - 1);
		else regionActors[i]->VisibilityOn();
	}

	renderer->ResetCameraClippingRange();
	Render();
}

void VolumePipeline::ToggleThresholdLabels() {
	SetThresholdLabels(!thresholdLabels);
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