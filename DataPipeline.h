#ifndef DataPipeline_H
#define DataPipeline_H

#include <string>
#include <vector>

#include <vtkSmartPointer.h>

class vtkImageData;

class DataPipeline {
public:
	DataPipeline();
	~DataPipeline();

	bool OpenImageFile(const std::string& fileName);
	bool OpenImageStack(const std::vector<std::string>& fileNames);

	bool OpenSegmentationFile(const std::string& fileName);
	bool OpenSegmentationStack(const std::vector<std::string>& fileNames);

	void SegmentVolume();
	bool SaveSegmentationData(const std::string& fileName);

	vtkImageData* GetData();
	vtkImageData* GetLabels();

protected:
	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;
};

#endif