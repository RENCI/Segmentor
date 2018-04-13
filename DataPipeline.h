#ifndef DataPipeline_H
#define DataPipeline_H

#include <string>

#include <vtkSmartPointer.h>

class vtkAlgorithm;

class DataPipeline {
public:

	DataPipeline();
	~DataPipeline();

	bool OpenData(const std::string& fileName);
	vtkAlgorithm* GetOutput();

protected:
	// The data
	vtkSmartPointer<vtkAlgorithm> reader;
	vtkSmartPointer<vtkAlgorithm> output;
};

#endif
