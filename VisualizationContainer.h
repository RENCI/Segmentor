#ifndef VisualizationContainer_H
#define VisualizationContainer_H

#include <string>
#include <vector>

#include <vtkSmartPointer.h>

class vtkImageData;
class vtkRenderWindowInteractor;

class DataPipeline;
class VolumePipeline;
class SlicePipeline;

class VisualizationContainer {
public:
	VisualizationContainer(vtkRenderWindowInteractor* volumeInteractor, vtkRenderWindowInteractor* sliceInteractor);
	~VisualizationContainer();

	bool OpenImageFile(const std::string& fileName);
	bool OpenImageStack(const std::vector<std::string>& fileNames);

	bool OpenSegmentationFile(const std::string& fileName);
	bool OpenSegmentationStack(const std::vector<std::string>& fileNames);

	void SegmentVolume();
	bool SaveSegmentationData(const std::string& fileName);

protected:
	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// Rendering pipelines
	VolumePipeline *volumePipeline;
	SlicePipeline *slicePipeline;
};

#endif