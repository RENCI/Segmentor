#ifndef DataPipeline_H
#define DataPipeline_H

#include <string>

#include <vtkSmartPointer.h>

class vtkImageData;

class DataPipeline {
public:
	DataPipeline();
	~DataPipeline();

	bool OpenData(const std::string& fileName);

	vtkImageData* GetData();
	vtkImageData* GetLabels();

protected:
	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;
};

#endif