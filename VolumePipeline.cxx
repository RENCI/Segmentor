#include "VolumePipeline.h"

#include <vtkActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkInteractorStyle.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <vtkOpenGLGPUVolumeRayCastMapper.h>

#include <vtkGlyph3DMapper.h>
#include <vtkGeometryFilter.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkThreshold.h>
#include <vtkLookupTable.h>

#include <vtkDiscreteMarchingCubes.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkProperty.h>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

vtkSmartPointer<vtkActor> VolumePipeline::CreateGeometry(vtkImageData* data) {
	// Get image info
	double minValue = data->GetScalarRange()[0];
	double maxValue = data->GetScalarRange()[1];
	
	int maxLabel = data->GetScalarRange()[1];

	// Contour
	contour->GenerateValues(maxLabel, 1, maxLabel);
	contour->SetInputDataObject(data);
	contour->ComputeNormalsOff();
	contour->ComputeGradientsOff();

	// Smoother
	int smoothingIterations = 15;
	double passBand = 0.0001;
	double featureAngle = 120.0;
	
	vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
	smoother->SetInputConnection(contour->GetOutputPort());
	smoother->SetNumberOfIterations(smoothingIterations);
	smoother->BoundarySmoothingOff();
	smoother->FeatureEdgeSmoothingOff();
	smoother->SetFeatureAngle(featureAngle);
	smoother->SetPassBand(passBand);
	smoother->NonManifoldSmoothingOn();
	smoother->NormalizeCoordinatesOn();

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

	// Mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetLookupTable(labelColors);
	mapper->UseLookupTableScalarRangeOn();
	mapper->SetInputConnection(smoother->GetOutputPort());

	// Actor
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	return actor;
}

vtkSmartPointer<vtkVolume> VolumePipeline::CreateVolume(vtkImageData* data) {
/*
	// Resize
	double mag[3] = { 1, 1, 1 };
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->GetMagnificationFactors(mag);
	resample->SetInputDataObject(data);
	resample->Update();

	double minValue = resample->GetOutput()->GetScalarRange()[0];
	double maxValue = resample->GetOutput()->GetScalarRange()[1];
*/
	// Get image info
	double minValue = data->GetScalarRange()[0];
	double maxValue = data->GetScalarRange()[1];

	double minVisible = rescale(0.05, minValue, maxValue);

	double gradientValue = 5.0;

	// Volume mapper
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToRayCast();
	volumeMapper->SetInputDataObject(data);

	// Volume property
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOn();
	volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

	// Scalar opacity transfer function
	/*
	vtkSmartPointer<vtkPiecewiseFunction> scalarOpacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	scalarOpacityFunction->AddPoint(minValue, 1.0);
	scalarOpacityFunction->AddPoint(minVisible, 1.0);
	scalarOpacityFunction->AddPoint(maxValue, 1.0);
	volumeProperty->SetScalarOpacity(scalarOpacityFunction);
	*/

	// Gradient opacity transfer function
	vtkSmartPointer<vtkPiecewiseFunction> gradientOpacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	gradientOpacityFunction->AddPoint(0, 0.0);
	gradientOpacityFunction->AddPoint(gradientValue, 0.0);
	gradientOpacityFunction->AddPoint(gradientValue + 1, 0.1);
	gradientOpacityFunction->AddPoint(maxValue - minValue, 0.1);
	volumeProperty->SetGradientOpacity(gradientOpacityFunction);

	// Color transfer function
	vtkSmartPointer<vtkColorTransferFunction> colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorFunction->AddRGBPoint(minValue, 1.0, 0.0, 0.0);
	colorFunction->AddRGBPoint(rescale(0.5, minValue, maxValue), 1.0, 1.0, 0.0);
	colorFunction->AddRGBPoint(maxValue, 1.0, 1.0, 1.0);
	volumeProperty->SetColor(colorFunction);

	// Volume
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	return volume;
}

vtkSmartPointer<vtkActor> labelGeometry(vtkImageData* labels) {
/*
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdByUpper(1);
	threshold->SetInputDataObject(labels);

	vtkSmartPointer<vtkGeometryFilter> geometry = vtkSmartPointer<vtkGeometryFilter>::New();
	geometry->SetInputConnection(threshold->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(geometry->GetOutputPort());
*/
/*
	vtkSmartPointer<vtkGlyph3DMapper> mapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
	mapper->MaskingOn();
	mapper->SetInputDataObject(labels);
*/
	vtkSmartPointer<vtkDiscreteMarchingCubes> cubes = vtkSmartPointer<vtkDiscreteMarchingCubes>::New();
	cubes->SetInputDataObject(labels);

	// Label colors
	vtkSmartPointer<vtkLookupTable> labelColors = vtkSmartPointer<vtkLookupTable>::New();
	labelColors->SetNumberOfTableValues(2);
	labelColors->SetRange(0.0, 1.0);
	labelColors->SetTableValue(0, 0.0, 0.0, 1.0, 1.0);	//label 0 is transparent
	labelColors->SetTableValue(1, 0.0, 1.0, 0.0, 1.0);	//label 1 is green
	labelColors->Build();

	// Mapper
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetLookupTable(labelColors);
	mapper->SetScalarModeToUseCellData();
	mapper->SetColorModeToMapScalars();
	mapper->SetInputConnection(cubes->GetOutputPort());

	// Actor
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	return actor;
}

/*
vtkSmartPointer<vtkVolume> labelVolume(vtkImageData* labels) {
	// Volume mapper
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToRayCast();
	volumeMapper->SetInputDataObject(labels);

	// Volume property
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

	// Scalar opacity transfer function
	vtkSmartPointer<vtkPiecewiseFunction> scalarOpacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	scalarOpacityFunction->AddPoint(0, 0.0);
	scalarOpacityFunction->AddPoint(1, 1.0);
	scalarOpacityFunction->ClampingOn();
	volumeProperty->SetScalarOpacity(scalarOpacityFunction);

	// Color transfer function
	vtkSmartPointer<vtkColorTransferFunction> colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
	colorFunction->AddRGBPoint(1.0, 0.0, 1.0, 0.0);
	colorFunction->ClampingOn();
	volumeProperty->SetColor(colorFunction);

	// Volume
	vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	return volume;
}
*/

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* rwi) {
	this->renderer = vtkSmartPointer<vtkRenderer>::New();
	this->contour = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

VolumePipeline::~VolumePipeline() {
}

void VolumePipeline::SetSegmentationData(vtkImageData* labels) {
	// Interaction
	reinterpret_cast<vtkInteractorStyle*>(renderer->GetRenderWindow()->GetInteractor()->GetInteractorStyle())->AutoAdjustCameraClippingRangeOff();

	// Render
	renderer->AddViewProp(CreateGeometry(labels));
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* VolumePipeline::GetRenderer() {
	return renderer;
}

vtkAlgorithmOutput* VolumePipeline::GetContour() {
	return contour->GetOutputPort();
}