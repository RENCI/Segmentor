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

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* interactor) {
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

	// Lookup table
	labelColors = vtkSmartPointer<vtkLookupTable>::New();

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

void VolumePipeline::SetSegmentationData(vtkImageData* data) {
	// Reset
	thresholdLabels = false;
	smoothSurfaces = true;
	currentLabel = 0;

	// Get label info
	int maxLabel = data->GetScalarRange()[1];;

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

	// Label colors
	labelColors->SetNumberOfTableValues(maxLabel + 1);
	labelColors->SetRange(0, maxLabel);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0);
	for (int i = 1; i <= maxLabel; i++) {
		double* c = colors[(i - 1) % numColors];
		labelColors->SetTableValue(i, c[0], c[1], c[2]);
	}
	labelColors->Build();

	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();

	ExtractRegions(data);

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
		labelColors->GetColor(label, color);
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

void VolumePipeline::ExtractRegions(vtkImageData* data) {
	// Get label info
	int maxLabel = data->GetScalarRange()[1];
	int numPoints = data->GetNumberOfPoints();
	unsigned short* scalars = static_cast<unsigned short*>(data->GetScalarPointer());

	int dataExtent[6];
	data->GetExtent(dataExtent);

	regionActors.clear();

	for (int label = 1; label <= maxLabel; label++) {
		// Initialize extent for this region
		int extent[6];
		extent[0] = dataExtent[1];
		extent[1] = dataExtent[0];
		extent[2] = dataExtent[3];
		extent[3] = dataExtent[2];
		extent[4] = dataExtent[5];
		extent[5] = dataExtent[4];

		for (int i = 0; i < numPoints; i++) {
			if (scalars[i] == label) {
				double* point = data->GetPoint(i);

				if (point[0] < extent[0]) extent[0] = point[0];
				if (point[0] > extent[1]) extent[1] = point[0];
				if (point[1] < extent[2]) extent[2] = point[1];
				if (point[1] > extent[3]) extent[3] = point[1];				
				if (point[2] < extent[4]) extent[4] = point[2];
				if (point[2] > extent[5]) extent[5] = point[2];
			}
		}

		// Add a buffer around VOI
		int padding = 2;
		extent[0] = std::max(dataExtent[0], extent[0] - padding);
		extent[1] = std::min(dataExtent[1], extent[1] + padding);
		extent[2] = std::max(dataExtent[2], extent[2] - padding);
		extent[3] = std::min(dataExtent[3], extent[3] + padding);
		extent[4] = std::max(dataExtent[4], extent[4] - padding);
		extent[5] = std::min(dataExtent[5], extent[5] + padding);
		
		vtkSmartPointer<vtkExtractVOI> voi = vtkSmartPointer<vtkExtractVOI>::New();
		voi->SetVOI(extent);
		voi->SetInputDataObject(data);

//		vtkSmartPointer<vtkContourFilter> contour = vtkSmartPointer<vtkContourFilter>::New();
//		vtkSmartPointer<vtkFlyingEdges3D> contour = vtkSmartPointer<vtkFlyingEdges3D>::New();
		vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New();
		contour->SetValue(0, label);
		contour->ComputeNormalsOff();
		contour->ComputeGradientsOff();
		contour->SetInputConnection(voi->GetOutputPort());
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
//		mapper->SetInputConnection(normals->GetOutputPort());
//		mapper->SetInputConnection(smoother->GetOutputPort());
		mapper->SetInputConnection(contour->GetOutputPort());
		mapper->ScalarVisibilityOff();

		double color[3];
		labelColors->GetColor(label, color);

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(color);
		actor->GetProperty()->SetDiffuse(1.0);
		actor->GetProperty()->SetAmbient(0.1);
		actor->GetProperty()->SetSpecular(0.0);
		
		regionActors.push_back(actor);

		renderer->AddActor(actor);
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