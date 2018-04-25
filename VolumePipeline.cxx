#include "VolumePipeline.h"

#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <vtkOpenGLGPUVolumeRayCastMapper.h>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* rwi) {
	this->renderer = vtkSmartPointer<vtkRenderer>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

VolumePipeline::~VolumePipeline() {
}

void VolumePipeline::SetInput(vtkImageData* input) {
	// Get image info
//	double minValue = input->GetScalarRange()[0];
//	double maxValue = input->GetScalarRange()[1];

	// Resize
	double mag[3] = { 1, 1, 1 };
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->GetMagnificationFactors(mag);
	resample->SetInputDataObject(input);
	resample->Update();

	double minValue = resample->GetOutput()->GetScalarRange()[0];
	double maxValue = resample->GetOutput()->GetScalarRange()[1];

	double minVisible = rescale(0.05, minValue, maxValue);

	double gradientValue = 5.0;

	// Volume mapper
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToRayCast();
	volumeMapper->SetInputConnection(resample->GetOutputPort());
  
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
	gradientOpacityFunction->AddPoint(gradientValue + 1, 1.0);
	gradientOpacityFunction->AddPoint(maxValue - minValue, 1.0);
	volumeProperty->SetGradientOpacity(gradientOpacityFunction);

	// Color transfer function
	vtkSmartPointer<vtkColorTransferFunction> colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorFunction->AddRGBPoint(minValue, 1.0, 0.0, 0.0);
	colorFunction->AddRGBPoint(rescale(0.5, minValue, maxValue), 1.0, 1.0, 0.0);
	colorFunction->AddRGBPoint(maxValue, 1.0, 1.0, 1.0);
	volumeProperty->SetColor(colorFunction);

	// Volume
	vtkSmartPointer<vtkVolume> volume =	vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	// Render
	renderer->AddViewProp(volume);
	renderer->ResetCamera();
	renderer->GetRenderWindow()->Render();
}

vtkRenderer* VolumePipeline::GetRenderer() {
	return renderer;
}