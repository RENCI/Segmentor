#include "VolumePipeline.h"

#include <vtkActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
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

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

vtkSmartPointer<vtkVolume> dataVolume(vtkImageData* data) {
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

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetLookupTable(labelColors);
	mapper->SetScalarModeToUseCellData();
	mapper->SetColorModeToMapScalars();
	mapper->SetInputConnection(cubes->GetOutputPort());

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

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

VolumePipeline::~VolumePipeline() {
}

void VolumePipeline::SetInput(vtkImageData* data, vtkImageData* labels) {
	// Render
//	renderer->AddViewProp(labelVolume(labels));
	renderer->AddViewProp(labelGeometry(labels));
	renderer->AddViewProp(dataVolume(data));
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* VolumePipeline::GetRenderer() {
	return renderer;
}