#include "DataPipeline.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>

DataPipeline::DataPipeline() {
	data = nullptr;
	labels = vtkSmartPointer<vtkImageData>::New();
}

DataPipeline::~DataPipeline() {
}

bool DataPipeline::OpenData(const std::string& fileName) {
	// Load data
	vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
	reader->SetFileName(fileName.c_str());
	reader->Update();

	// Set output
	data = reader->GetOutput();

	// Create label volume
	labels->CopyStructure(data);
	labels->AllocateScalars(VTK_UNSIGNED_INT, 1);

	int* dims = labels->GetDimensions();

	for (int z = 0; z < dims[2]; z++) {
		for (int y = 0; y < dims[1]; y++) {
			for (int x = 0; x < dims[0]; x++) {
				static_cast<unsigned int*>(labels->GetScalarPointer(x, y, z))[0] = 0;
			}
		}
	}

	return true;
}

vtkImageData* DataPipeline::GetData() {
	return data;
}

vtkImageData* DataPipeline::GetLabels() {
	return labels;
}