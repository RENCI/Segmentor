#ifndef SliceView_H
#define SliceView_H

#include <vtkSmartPointer.h>

#include "InteractionEnums.h"

class vtkActor;
class vtkCylinderSource;
class vtkImageData;
class vtkImageSlice;
class vtkLookupTable;
class vtkObject;
class vtkPlane;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkTextActor;

class vtkInteractorStyleSlice;

class Brush;
class Probe;
class Region;
class RegionOutline;
class RegionCollection;
class SliceLocation;

class SliceView {
public:
	SliceView(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut);
	~SliceView();

	void Reset();

	void SetImageData(vtkImageData* data);
	void SetSegmentationData(vtkImageData* data, RegionCollection* newRegions);
	void AddRegion(Region* region);

	void UpdateVoxelSize();

	void SetShowProbe(bool show);
	void SetProbePosition(double x, double y, double z);

	void SetInteractionMode(enum InteractionMode mode);
	void SetFilterMode(enum FilterMode mode);

	void SetCurrentRegion(Region* region);

	void ShowRegion(Region* region, bool show = true);
	
	bool GetShowLabelSlice();
	void ShowLabelSlice(bool show);
	void ToggleLabelSlice();

	bool GetShowRegionOutlines();
	void ShowRegionOutlines(bool show);
	void ToggleRegionOutlines();

	double GetWindow();
	double GetLevel();
	void SetWindow(double window);
	void SetLevel(double level);

	void RescaleFull();
	void RescalePartial();
	bool GetAutoRescale();
	void SetAutoRescale(bool rescale);

	double GetOverlayOpacity();
	void SetOverlayOpacity(double opacity);

	void UpdatePlane();

	void SetBrushRadius(int radius);

	void SetOrientationX();
	void SetOrientationY();
	void SetOrientationZ();

	void Render();

	vtkSmartPointer<vtkRenderer> GetRenderer();
	vtkSmartPointer<vtkInteractorStyleSlice> GetInteractorStyle();

protected:
	FilterMode filterMode;
	bool showRegionOutlines;

	enum RescaleMode {
		Full,
		Partial
	};

	RescaleMode rescaleMode;
	bool autoRescale;

	Region* currentRegion;

	// Data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkRenderer> labelSliceRenderer;
	vtkSmartPointer<vtkRenderer> regionOutlinesRenderer;

	vtkSmartPointer<vtkInteractorStyleSlice> style;
	vtkSmartPointer<vtkLookupTable> labelColors;

	// Regions
	RegionCollection* regions;

	// Cut plane
	vtkSmartPointer<vtkPlane> plane;

	// Slices
	vtkSmartPointer<vtkImageSlice> slice;
	vtkSmartPointer<vtkImageSlice> labelSlice;

	// Probe
	Probe* probe;

	// Brush
	Brush* brush;

	// Slice location
	SliceLocation* sliceLocation;

	// Interaction mode label
	vtkSmartPointer<vtkTextActor> interactionModeLabel;
	void CreateInteractionModeLabel();

	// Slices
	void CreateSlice();
	void UpdateSlice();

	void CreateLabelSlice();
	void UpdateLabelSlice();

	void AddRegionActors(Region* region);
	void FilterRegions();

	void ResetCamera();

	void DoAutoRescale();

	void SetOrientation(const double leftToRight[3], const double viewUp[3]);

	vtkSmartPointer<vtkImageData> GetSlice();

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
