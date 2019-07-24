#include "DataPipeline.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>

DataPipeline::DataPipeline() {
	data = nullptr;
	labels = nullptr;
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenImageFile(const std::string& fileName) {
	double zScale = 2.0;

	// Load data
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();

	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		info->SetInputConnection(reader->GetOutputPort());
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		info->SetInputConnection(reader->GetOutputPort());
	}
	else {
		return false;
	}

	// Match output dims and resolution
	info->SetOutputSpacing(1.0, 1.0, zScale);

	// Resample in z
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->SetInputConnection(info->GetOutputPort()); 
	resample->SetAxisMagnificationFactor(2, zScale);
	resample->Update();

	// Set data output
	data = resample->GetOutput();

	return true;
}

bool DataPipeline::OpenImageStack(const std::vector<std::string>& fileNames) {
	return false;
}

bool DataPipeline::OpenSegmentationFile(const std::string& fileName) {
	//double zScale = 2.0;

	// Load data
	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	reader->SetFileName(fileName.c_str());

	// Match output dims and resolution
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetInputConnection(reader->GetOutputPort());
	//info->SetOutputSpacing(1.0, 1.0, zScale);
	info->Update();

/*
	// Resample in z
	vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
	resample->SetInputConnection(info->GetOutputPort());
	resample->SetAxisMagnificationFactor(2, zScale);
	resample->Update();
*/

	// Set label output
	labels = info->GetOutput();

	return true;
}

bool DataPipeline::OpenSegmentationStack(const std::vector<std::string>& fileNames) {
	return false;
}

bool DataPipeline::SaveSegmentationData(const std::string& fileName) {
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else if (extension == "tif") {

	}
	else {
		return false;
	}
	
	return true;
}

void DataPipeline::SegmentVolume() {
	if (!data) return;

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
	threshold->SetInputDataObject(data);

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
	labels = connectivity->GetOutput();
}

vtkImageData* DataPipeline::GetData() {
	return data;
}

vtkImageData* DataPipeline::GetLabels() {
	return labels;
}