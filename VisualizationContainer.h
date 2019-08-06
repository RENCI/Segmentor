#ifndef VisualizationContainer_H
#define VisualizationContainer_H

#include <string>
#include <vector>

#include <vtkSmartPointer.h>

class vtkImageData;
class vtkLookupTable;
class vtkRenderWindowInteractor;

class VolumePipeline;
class SlicePipeline;
class Region;

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

	void PickLabel(int x, int y, int z);
	void Paint(int x, int y, int z);
	void Erase(int x, int y, int z);

	void PickPointLabel(double x, double y, double z);
	void PaintPoint(double x, double y, double z);
	void ErasePoint(double x, double y, double z);

	void SetCurrentLabel(unsigned short newLabel);

	void Render();

	VolumePipeline* GetVolumePipeline();
	SlicePipeline* GetSlicePipeline();

protected:
	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	std::vector<Region*> regions;
	unsigned short currentLabel;
	vtkSmartPointer<vtkLookupTable> labelColors;

	// Rendering pipelines
	VolumePipeline *volumePipeline;
	SlicePipeline *slicePipeline;

	void UpdateLabels();
	void UpdateColors();

	void RemoveRegions();
	void ExtractRegions();

	void SetLabel(int x, int y, int z, unsigned short label);
	void SetPointLabel(double x, double y, double z, unsigned short label);
};

#endif