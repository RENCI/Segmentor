#include "DataPipeline.h"

#include <vtkAlgorithm.h>
#include <vtkXMLImageDataReader.h>

#include <vtkSphereSource.h>

DataPipeline::DataPipeline() {
	output = nullptr;
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenData(const std::string& fileName) {
	std::cout << fileName << std::endl;

	output = vtkSmartPointer<vtkSphereSource>::New();

	return true;
}

vtkAlgorithm* DataPipeline::GetOutput() {
	return output;
}