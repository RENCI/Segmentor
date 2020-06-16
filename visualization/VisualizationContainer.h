#ifndef VisualizationContainer_H
#define VisualizationContainer_H

#include <string>
#include <vector>

#include <vtkSmartPointer.h>

#include "InteractionEnums.h"

class MainWindow;

class vtkImageData;
class vtkLookupTable;
class vtkRenderWindowInteractor;

class VolumeView;
class SliceView;
class Region;
class RegionCollection;

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
		VolumeMismatch,
		NoFileName
	};

	FileErrorCode OpenImageFile(const std::string& fileName);
	FileErrorCode OpenImageStack(const std::vector<std::string>& fileNames);

	FileErrorCode OpenSegmentationFile(const std::string& fileName);
	FileErrorCode OpenSegmentationStack(const std::vector<std::string>& fileNames);

	FileErrorCode SaveImageData(const std::string& fileName);

	void SegmentVolume();
	FileErrorCode SaveSegmentationData();
	FileErrorCode SaveSegmentationData(const std::string& fileName);

	void SetVoxelSize(double x, double y, double z);

	enum InteractionMode GetInteractionMode();
	void SetInteractionMode(enum InteractionMode mode);
	void ToggleInteractionMode();

	enum FilterMode GetFilterMode();
	void SetFilterMode(enum FilterMode mode);

	void PickLabel(double point[3]);
	void Paint(double point[3]);
	void Erase(double point[3]);

	void Paint(int i, int j, int k);
	void Erase(int i, int j, int );

	void SetProbePosition(double point[3]);

	//void PickPointLabel(double x, double y, double z);
	//void PaintPoint(double x, double y, double z);
	//void ErasePoint(double x, double y, double z);

	void SetCurrentRegion(Region* region);

	void RemoveRegion(unsigned short label);
	void SetRegionDone(unsigned short label, bool done);
	void HighlightRegion(unsigned short label);
	void SelectRegion(unsigned short label);

	void RelabelCurrentRegion();
	void MergeWithCurrentRegion(double point[3]);
	void SplitCurrentRegion(int numRegions);
	void DilateCurrentRegion();
	void ErodeCurrentRegion();
	void GrowCurrentRegion(double point[3]);

	void SetWindowLevel(double window, double level);

	void SliceUp();
	void SliceDown();

	void SetFocalPoint(double x, double y, double z);

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

	// Current filter mode
	FilterMode filterMode;

	// Current segmentation data filename
	std::string segmentationDataFileName;

	void SetImageData(vtkImageData* imageData);
	bool SetLabelData(vtkImageData* labelData);

	void UpdateLabels();
	void UpdateColors();
	void UpdateColors(unsigned short label);

	void ExtractRegions();

	void DoDilateErode(bool doDilate);

	// Region metadata
	void LoadRegionMetadata(std::string fileName);
	void SaveRegionMetadata(std::string fileName);

	void SetLabel(int x, int y, int z, unsigned short label);
	unsigned short GetLabel(int x, int y, int z);

	double GetValue(int x, int y, int z);

	void SliceStep(double amount);

	void PointToIndex(double point[3], int ijk[3]);
	void IndexToPoint(int ijk[3], double point[3]);

	//void PointToStructured(double p[3], int s[3]);
};

#endif