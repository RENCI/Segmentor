#include "DataPipeline.h"

#include <vtkXMLImageDataReader.h>

DataPipeline::DataPipeline() {
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenData(const std::string& fileName) {
	return true;
}