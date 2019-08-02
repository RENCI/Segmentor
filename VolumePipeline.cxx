#include "VolumePipeline.h"

#include "vtkInteractorStyleVolume.h"

#include <vtkActor.h>

#include <vtkCubeSource.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>
#include <vtkInteractorStyle.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include <vtkContourFilter.h>
#include <vtkExtractVOI.h>
#include <vtkPolyDataNormals.h>

#include <vtkLight.h>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* interactor) {
	thresholdLabels = false;
	smoothSurfaces = true;

	// Rendering
	renderer = vtkSmartPointer<vtkRenderer>::New();
	interactor->GetRenderWindow()->AddRenderer(renderer);

	// Interaction
	style = vtkSmartPointer<vtkInteractorStyleVolume>::New();

	interactor->SetInteractorStyle(style);
	interactor->SetNumberOfFlyFrames(5);

	// Pipeline
	CreatePipeline();
	UpdatePipeline();

	// Probe
	CreateProbe();

/*
	double lightPosition[3] = { 0, 0.5, 1 };

	// Create a light
	double lightFocalPoint[3] = { 0, 0, 0 };

	vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	light->SetLightTypeToCameraLight();
	light->SetPosition(lightPosition[0], lightPosition[1], lightPosition[2]);
	light->SetFocalPoint(lightFocalPoint[0], lightFocalPoint[1], lightFocalPoint[2]);
	light->SetDiffuseColor(1, 1, 1);
	light->SetAmbientColor(1, 1, 1);
	light->SetSpecularColor(1, 1, 1);

	renderer->AddLight(light);
*/
}

VolumePipeline::~VolumePipeline() {
}

void VolumePipeline::SetSegmentationData(vtkImageData* data) {
	// Reset
	thresholdLabels = false;
	smoothSurfaces = true;

	threshold->SetInputDataObject(data);

	// Get label info
	int maxLabel = data->GetScalarRange()[1];

	// Contour
	contour->GenerateValues(maxLabel, 1, maxLabel);
	contour->SetInputDataObject(data);

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
	vtkSmartPointer<vtkLookupTable> labelColors = vtkSmartPointer<vtkLookupTable>::New();
	labelColors->SetNumberOfTableValues(maxLabel + 1);
	labelColors->SetRange(0, maxLabel);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0);
	for (int i = 1; i <= maxLabel; i++) {
		double* c = colors[(i - 1) % numColors];
		labelColors->SetTableValue(i, c[0], c[1], c[2]);
	}
	labelColors->Build();

	mapper->SetLookupTable(labelColors);

	// Update probe
	UpdateProbe(data);
	probe->VisibilityOn();
//	renderer->AddActor(actor);

	ExtractRegions(data, labelColors);

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

	UpdatePipeline();
	Render();
}

void VolumePipeline::ToggleSmoothSurfaces() {
	SetSmoothSurfaces(!smoothSurfaces);
}

void VolumePipeline::SetLabel(unsigned short label) {
	if (label > 0) {
		double color[3];
		mapper->GetLookupTable()->GetColor(label, color);
		probe->GetProperty()->SetColor(color);
	}
	else {
		probe->GetProperty()->SetColor(1, 1, 1);
	}

	threshold->ThresholdBetween(label, label);
}

void VolumePipeline::SetThresholdLabels(bool doThreshold) {
	thresholdLabels = doThreshold;

	UpdatePipeline();


	// XXX: HACK
	for (int i = 0; i < regionActors.size(); i++) {
		if (thresholdLabels) regionActors[i]->SetVisibility(i == threshold->GetLowerThreshold() - 1);
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

vtkAlgorithmOutput* VolumePipeline::GetContour() {
	return contour->GetOutputPort();
}

void VolumePipeline::CreatePipeline() {
	// Threshold
	threshold = vtkSmartPointer<vtkImageThreshold>::New();
	threshold->ReplaceInOff();
	threshold->ReplaceOutOn();
	threshold->SetOutValue(0);

	// Contour
	contour = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New(); 
	contour->ComputeNormalsOff();
	contour->ComputeGradientsOff();

	// Smoother
	int smoothingIterations = 10;
	double passBand = 0.01;
	double featureAngle = 120.0;

	smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
	smoother->SetNumberOfIterations(smoothingIterations);
	//smoother->BoundarySmoothingOff();
	//smoother->FeatureEdgeSmoothingOff();
	//smoother->SetFeatureAngle(featureAngle);
	smoother->SetPassBand(passBand);
	//smoother->NonManifoldSmoothingOn();
	smoother->NormalizeCoordinatesOn();
	smoother->SetInputConnection(contour->GetOutputPort());

	// Mapper
	mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->UseLookupTableScalarRangeOn();

	// Actor
	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
}

void VolumePipeline::UpdatePipeline() {
	if (thresholdLabels) {
		contour->SetInputConnection(threshold->GetOutputPort());
	}
	else {
		contour->SetInputDataObject(threshold->GetInputDataObject(0, 0));
	}

	if (smoothSurfaces) {
		mapper->SetInputConnection(smoother->GetOutputPort());
	}
	else {
		mapper->SetInputConnection(contour->GetOutputPort());
	}
}

void VolumePipeline::ExtractRegions(vtkImageData* data, vtkLookupTable* lut) {
	// Get label info
	int maxLabel = data->GetScalarRange()[1];
	int numPoints = data->GetNumberOfPoints();
	unsigned short* scalars = static_cast<unsigned short*>(data->GetScalarPointer());

	regionActors.clear();

	for (int label = 1; label <= maxLabel; label++) {
		int extent[6];
		extent[0] = data->GetExtent()[1];
		extent[1] = data->GetExtent()[0];
		extent[2] = data->GetExtent()[3];
		extent[3] = data->GetExtent()[2];
		extent[4] = data->GetExtent()[5];
		extent[5] = data->GetExtent()[4];

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
		extent[0] = extent[0] == data->GetExtent()[0] ? extent[0] : extent[0] - 1;
		extent[1] = extent[1] == data->GetExtent()[1] ? extent[1] : extent[1] + 1;
		extent[2] = extent[2] == data->GetExtent()[2] ? extent[2] : extent[2] - 1;
		extent[3] = extent[3] == data->GetExtent()[3] ? extent[3] : extent[3] + 1;
		extent[4] = extent[4] == data->GetExtent()[4] ? extent[4] : extent[4] - 1;
		extent[5] = extent[5] == data->GetExtent()[5] ? extent[5] : extent[5] + 1;

		vtkSmartPointer<vtkExtractVOI> voi = vtkSmartPointer<vtkExtractVOI>::New();
		voi->SetInputDataObject(data);
		voi->SetVOI(extent);

		vtkSmartPointer<vtkContourFilter> contour = vtkSmartPointer<vtkContourFilter>::New();
		contour->SetValue(0, label);
		contour->ComputeNormalsOff();
		contour->SetInputConnection(voi->GetOutputPort());

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
		//normals->SetInputConnection(contour->GetOutputPort());
		normals->SetInputConnection(smoother->GetOutputPort());


		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		//mapper->SetInputConnection(normals->GetOutputPort());
		mapper->SetInputConnection(smoother->GetOutputPort());
		//mapper->SetInputConnection(contour->GetOutputPort());
		mapper->ScalarVisibilityOff();

		double color[3];
		lut->GetColor(label, color);

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		actor->GetProperty()->SetColor(color);
		//actor->GetProperty()->SetAmbient(0.2);
		//actor->GetProperty()->SetInterpolationToPhong();

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