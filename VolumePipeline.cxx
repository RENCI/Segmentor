#include "VolumePipeline.h"

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

double rescale(double value, double min, double max) {
	return min + (max - min) * value;
}

VolumePipeline::VolumePipeline(vtkRenderWindowInteractor* rwi) {
	this->renderer = vtkSmartPointer<vtkRenderer>::New();

	rwi->GetRenderWindow()->AddRenderer(renderer);
}

VolumePipeline::~VolumePipeline() {
}

void VolumePipeline::SetInput(vtkAlgorithm* input) {
	double minValue = 10.0;
	double maxValue = 255.0;

	// Volume mapper
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volumeMapper->SetBlendModeToComposite();
	volumeMapper->SetRequestedRenderModeToRayCast();
	volumeMapper->SetInputConnection(input->GetOutputPort());
  
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