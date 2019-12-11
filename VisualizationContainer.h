#ifndef VisualizationContainer_H
#define VisualizationContainer_H

#include <string>
#include <vector>

#include <vtkSmartPointer.h>

class MainWindow;

class vtkImageData;
class vtkLookupTable;
class vtkRenderWindowInteractor;

class VolumeView;
class SliceView;
class Region;
class RegionCollection;

enum InteractionMode;

class VisualizationContainer {
public:
	VisualizationContainer(
		vtkRenderWindowInteractor* volumeInteractor, 
		vtkRenderWindowInteractor* sliceInteractor,
		MainWindow* mainWindow);
	~VisualizationContainer();

	enum FileErrorCode {
		Success = 1,
		WrongFileType,
		NoImageData,
		VolumeMismatch
	};

	FileErrorCode OpenImageFile(const std::string& fileName);
	FileErrorCode OpenImageStack(const std::vector<std::string>& fileNames);

	FileErrorCode OpenSegmentationFile(const std::string& fileName);
	FileErrorCode OpenSegmentationStack(const std::vector<std::string>& fileNames);

	void SegmentVolume();
	FileErrorCode SaveSegmentationData(const std::string& fileName);

	void SetInteractionMode(InteractionMode mode);
	void ToggleInteractionMode();

	void PickLabel(int x, int y, int z);
	void Paint(int x, int y, int z);
	void Erase(int x, int y, int z);

	void PickPointLabel(double x, double y, double z);
	void PaintPoint(double x, double y, double z);
	void ErasePoint(double x, double y, double z);

	void SetCurrentRegion(Region* region);

	void RelabelCurrentRegion();
	void MergeWithCurrentRegion(int x, int y, int z);

	void GrowRegion(int x, int y, int z);
	void RemoveRegion(unsigned short label);
	void SetRegionDone(unsigned short label, bool done);

	void Render();

	VolumeView* GetVolumeView();
	SliceView* GetSliceView();

protected:
	// Qt main window
	MainWindow* qtWindow;

	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// Regions
	RegionCollection* regions;
	Region* currentRegion;

	vtkSmartPointer<vtkLookupTable> labelColors;

	// Rendering pipelines
	VolumeView *volumeView;
	SliceView *sliceView;

	// Current interaction mode
	InteractionMode interactionMode;

	void SetImageData(vtkImageData* imageData);
	bool SetLabelData(vtkImageData* labelData);

	void UpdateLabels();
	void UpdateColors();
	void UpdateColors(unsigned short label);

	void ExtractRegions();

	// Region metadata
	void LoadRegionMetadata(std::string fileName);
	void SaveRegionMetadata(std::string fileName);

	void SetLabel(int x, int y, int z, unsigned short label);
	unsigned short GetLabel(int x, int y, int z);

	double GetValue(int x, int y, int z);

	void PointToStructured(double p[3], int s[3]);
};

#endif