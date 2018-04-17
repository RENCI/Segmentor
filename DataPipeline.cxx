#include "DataPipeline.h"

#include <vtkAlgorithm.h>
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

	// Set output
	output = reader;

	return true;
}

vtkAlgorithm* DataPipeline::GetOutput() {
	return output;
}