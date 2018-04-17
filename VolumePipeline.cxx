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
	double minValue = input->GetScalarRange()[0];
	double maxValue = input->GetScalarRange()[1];

	// Resize
	double mag[3] = { 0.1, 0.1, 0.1 };
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->GetMagnificationFactors(mag);
	resample->SetInputDataObject(input);

	// Volume mapper
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToRayCast();
	volumeMapper->SetInputConnection(resample->GetOutputPort());
  
	// Volume property
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

	// Opacity transfer function
	vtkSmartPointer<vtkPiecewiseFunction> opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityFunction->AddPoint(0.0, 0.0);
	opacityFunction->AddPoint(minValue, 0.0);
	opacityFunction->AddPoint(maxValue, 1.0);
	volumeProperty->SetScalarOpacity(opacityFunction);

	// Color transfer function
	vtkSmartPointer<vtkColorTransferFunction> colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	colorFunction->AddRGBPoint(minValue, 0.0, 0.0, 0.0);
	colorFunction->AddRGBPoint(rescale(1.0 / 3.0, minValue, maxValue), 1.0, 0.0, 0.0);
	colorFunction->AddRGBPoint(rescale(2.0 / 3.0, minValue, maxValue), 1.0, 1.0, 0.0);
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