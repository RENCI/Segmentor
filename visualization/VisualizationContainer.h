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
class History;

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

	void InitializeLabelData();

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
	void Paint(double point[3], bool overwrite = false);
	void Erase(double point[3]);

	void Paint(int i, int j, int k, bool overwrite = false, bool useBrush = true);
	void Erase(int i, int j, int k, bool useBrush = true);

	void MouseMove();
	void MouseMove(double point[3]);

	//void PickPointLabel(double x, double y, double z);
	//void PaintPoint(double x, double y, double z);
	//void ErasePoint(double x, double y, double z);

	void SetCurrentRegion(Region* region);

	bool GetFilterRegions();
	void SetFilterRegions(bool filter);

	void ClearRegionVisibilities();
	void ShowPlaneRegions();
	void ShowNeighborRegions();
	
	void ToggleRegionVisibility(double point[3]);
	void ToggleRegionVisibility(unsigned short label);

	void SetVisibleOpacity(double opacity);

	void RemoveRegion(unsigned short label);
	Region* SetRegionDone(unsigned short label, bool done);
	void ToggleRegionDone(double point[3]);
	void HighlightRegion(unsigned short label);
	void SelectRegion(unsigned short label, bool flyTo = true);
	void SetRegionVisibility(unsigned short label, bool visible);
	void SetRegionColor(unsigned short label, double r, double g, double b);

	void CreateNewRegion(double point[3]);
	void RelabelCurrentRegion();
	void MergeWithCurrentRegion(double point[3]);
	void SplitCurrentRegion(int numRegions);
	void GrowCurrentRegion(double point[3]);

	void SetWindowLevel(double window, double level);

	void SliceUp();
	void SliceDown();

	void SetFocalPoint(double x, double y, double z);

	void SetBrushRadius(int radius);

	void Render();

	void Undo();
	void Redo();

	bool NeedToSave();

	VolumeView* GetVolumeView();
	SliceView* GetSliceView();

	const double* GetDataRange();

protected:
	// Qt main window
	MainWindow* qtWindow;

	// The data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// History
	History* history;
	int numEdits;

	// Regions
	RegionCollection* regions;
	Region* currentRegion;
	unsigned short hoverLabel;

	bool filterRegions;

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

	// Brush radius
	int brushRadius;

	void SetImageData(vtkImageData* imageData);
	bool SetLabelData(vtkImageData* labelData);

	void UpdateLabels();
	void UpdateColors();
	void UpdateColors(unsigned short label);

	void ExtractRegions();

	void SplitRegionKMeans(Region* region, int numRegions);
	void SplitRegionIntensity(Region* region, int numRegions);

	// Region metadata
	void LoadRegionMetadata(std::string fileName);
	void SaveRegionMetadata(std::string fileName);

	int SetLabel(int x, int y, int z, unsigned short label, bool overwrite = false);
	unsigned short GetLabel(int x, int y, int z);

	double GetValue(int x, int y, int z);

	void SliceStep(double amount);

	void PointToIndex(double point[3], int ijk[3]);
	void IndexToPoint(int ijk[3], double point[3]);

	bool InBounds(int ijk[3]);

	void PushHistory();
	void PopHistory();

	void UpdateVisibility(Region* highlightRegion = nullptr);
};

#endif