#include "DataPipeline.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>

DataPipeline::DataPipeline() {
	output = nullptr;
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenData(const std::string& fileName) {
	// Load data
	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	reader->SetFileName(fileName.c_str());
	reader->Update();

	// Set output
	output = reader->GetOutput();

	return true;
}

vtkImageData* DataPipeline::GetOutput() {
	return output;
}