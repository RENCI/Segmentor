#ifndef DataPipeline_H
#define DataPipeline_H

#include <string>

#include <vtkSmartPointer.h>

class vtkAlgorithm;
class vtkAlgorithmOutput;

class DataPipeline {
public:

	DataPipeline();
	~DataPipeline();

	bool OpenData(const std::string& fileName);
	vtkAlgorithmOutput* GetData();

protected:
	// The data
	vtkSmartPointer<vtkAlgorithm> reader;
	vtkSmartPointer<vtkAlgorithmOutput> data;
};

#endif
