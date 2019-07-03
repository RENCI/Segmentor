#include "DataPipeline.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageResample.h>

DataPipeline::DataPipeline() {
	data = nullptr;
	labels = vtkSmartPointer<vtkImageData>::New();
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenData(const std::string& fileName) {
	double zScale = 2.0;

	// Load data
	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	reader->SetFileName(fileName.c_str());

	// Match output dims and resolution
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetInputConnection(reader->GetOutputPort());
	info->SetOutputSpacing(1.0, 1.0, zScale);

	// Resample in z
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->SetInputConnection(info->GetOutputPort()); 
	resample->SetAxisMagnificationFactor(2, zScale);
	resample->Update();

	// Set data output
	this->data = resample->GetOutput();

	this->data->PrintSelf(std::cout, vtkIndent(2));

	// Generate labels
	double minValue = data->GetScalarRange()[0];
	double maxValue = data->GetScalarRange()[1];

	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange((maxValue - minValue) * 0.3, maxValue);
	connectivity->SetInputConnection(resample->GetOutputPort());
	connectivity->Update();
	
	// Set label output
	this->labels = connectivity->GetOutput();

	this->labels->PrintSelf(std::cout, vtkIndent(2));

	return true;
}

vtkImageData* DataPipeline::GetData() {
	return data;
}

vtkImageData* DataPipeline::GetLabels() {
	return labels;
}