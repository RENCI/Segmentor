#include "VolumePipeline.h"

#include "vtkInteractorStyleVolume.h"

#include <vtkActor.h>

#include <vtkCubeSource.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkInteractorStyle.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkThreshold.h>
#include <vtkWindowedSincPolyDataFilter.h>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* interactor) {
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
}

VolumePipeline::~VolumePipeline() {
}

void VolumePipeline::SetSegmentationData(vtkImageData* data) {
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

	// Render
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
	renderer->AddActor(actor);
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
	if (smoothSurfaces) {
		mapper->SetInputConnection(smoother->GetOutputPort());
	}
	else {
		mapper->SetInputConnection(contour->GetOutputPort());
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