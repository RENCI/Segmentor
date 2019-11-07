#ifndef VisualizationContainer_H
#define VisualizationContainer_H

#include <string>
#include <vector>

#include <vtkSmartPointer.h>

class MainWindow;

class vtkImageData;
class vtkLookupTable;
class vtkRenderWindowInteractor;

class VolumePipeline;
class SlicePipeline;
class Region;

class VisualizationContainer {
public:
	VisualizationContainer(
		vtkRenderWindowInteractor* volumeInteractor, 
		vtkRenderWindowInteractor* sliceInteractor,
		MainWindow* mainWindow);
	~VisualizationContainer();

	bool OpenImageFile(const std::string& fileName);
	bool OpenImageStack(const std::vector<std::string>& fileNames);

	bool OpenSegmentationFile(const std::string& fileName);
	bool OpenSegmentationStack(const std::vector<std::string>& fileNames);

	void SegmentVolume();
	bool SaveSegmentationData(const std::string& fileName);

	void ToggleInteractionMode();

	void PickLabel(int x, int y, int z);
	void Paint(int x, int y, int z);
	void Erase(int x, int y, int z);

	void PickPointLabel(double x, double y, double z);
	void PaintPoint(double x, double y, double z);
	void ErasePoint(double x, double y, double z);

	void SetCurrentLabel(unsigned short newLabel);

	void RelabelCurrentRegion();
	void MergeWithCurrentRegion(int x, int y, int z);

	void GrowRegion(int x, int y, int z);
	void RemoveRegion(unsigned short label);
	void SetRegionDone(unsigned short label, bool done);

	void Render();

	VolumePipeline* GetVolumePipeline();
	SlicePipeline* GetSlicePipeline();

protected:
	// Qt main window
	MainWindow* qtWindow;

	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	std::vector<Region*> regions;
	Region* currentRegion;

	vtkSmartPointer<vtkLookupTable> labelColors;

	// Rendering pipelines
	VolumePipeline *volumePipeline;
	SlicePipeline *slicePipeline;

	// Current interaction mode
	enum InteractionMode interactionMode;

	void UpdateLabels();
	void UpdateColors();
	void UpdateColors(unsigned short label);

	unsigned short GetNewLabel();

	int GetRegionIndex(unsigned short label);
	Region* GetRegion(unsigned short label);
	void RemoveRegions();
	void ExtractRegions();

	void SetLabel(int x, int y, int z, unsigned short label);
	unsigned short GetLabel(int x, int y, int z);

	double GetValue(int x, int y, int z);

	void PointToStructured(double p[3], int s[3]);
};

#endif