#ifndef DataPipeline_H
#define DataPipeline_H

#include <string>

#include <vtkSmartPointer.h>

class vtkAlgorithm;
class vtkImageData;

class DataPipeline {
public:
	DataPipeline();
	~DataPipeline();

	bool OpenData(const std::string& fileName);
	vtkImageData* GetOutput();

protected:
	// The data
	vtkSmartPointer<vtkImageData> output;
};

#endif