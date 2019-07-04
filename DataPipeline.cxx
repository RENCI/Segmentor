#include "DataPipeline.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>

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

	// Filter
	double minValue = data->GetScalarRange()[0];
	double maxValue = data->GetScalarRange()[1];

	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
	threshold->ThresholdByUpper(minValue + (maxValue - minValue) * 0.15);
	threshold->SetInValue(255);
	threshold->SetOutValue(0);
	threshold->ReplaceInOn();
	threshold->ReplaceOutOn();
	threshold->SetOutputScalarTypeToUnsignedChar();
	threshold->SetInputConnection(resample->GetOutputPort());

	vtkSmartPointer<vtkImageOpenClose3D> openClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
	openClose->SetKernelSize(10, 10, 10);
	openClose->SetOpenValue(0);
	openClose->SetCloseValue(255);
	openClose->SetInputConnection(threshold->GetOutputPort());

	// Generate labels
	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange(255, 255);
	connectivity->SetLabelScalarTypeToInt();
	connectivity->SetInputConnection(openClose->GetOutputPort());
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