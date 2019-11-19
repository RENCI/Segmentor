#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class SliceLocation;

class vtkActor;
class vtkAlgorithmOutput;
class vtkContourFilter;
class vtkImageData;
class vtkImageSlice;
class vtkLookupTable;
class vtkObject;
class vtkPlane;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkTextActor;

class vtkInteractorStyleSlice;

class Region;
class RegionOutline;
class RegionCollection;

class SlicePipeline {
public:
	SlicePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut);
	~SlicePipeline();

	void SetImageData(vtkImageData* data);
	void SetSegmentationData(vtkImageData* data, RegionCollection* newRegions);

	void RemoveRegion(Region* region);

	void SetShowProbe(bool show);
	void SetProbePosition(double x, double y, double z);

	void SetInteractionMode(enum InteractionMode mode);

	void SetCurrentRegion(Region* region);

	void SetCurrentLabel(unsigned short label);
	
	void ToggleLabelSlice();
	void ToggleLabelOutlines();

	void ShowRegionOutlines(bool show);
	void ToggleRegionOutlines();

	void SetFilterRegion(bool filter);
	void ToggleFilterRegion();

	void UpdatePlane();

	void Render();

	vtkSmartPointer<vtkRenderer> GetRenderer();
	vtkSmartPointer<vtkInteractorStyleSlice> GetInteractorStyle();

protected:
	bool showRegionOutlines;
	bool filterRegion;

	Region* currentRegion;

	// Data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkRenderer> labelSliceRenderer;
	vtkSmartPointer<vtkRenderer> labelOutlinesRenderer;
	vtkSmartPointer<vtkRenderer> regionOutlinesRenderer;

	vtkSmartPointer<vtkInteractorStyleSlice> style;
	vtkSmartPointer<vtkLookupTable> labelColors;

	// Regions
	RegionCollection* regions;

	// Cut plane
	vtkSmartPointer<vtkPlane> plane;

	// Overlays
	vtkSmartPointer<vtkImageSlice> labelSlice;
	vtkSmartPointer<vtkActor> labelOutlines;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	// Slice location
	SliceLocation* sliceLocation;

	// Interaction mode label
	vtkSmartPointer<vtkTextActor> interactionModeLabel;
	void CreateInteractionModeLabel();

	// Slices
	void CreateDataSlice(vtkImageData* data);
	void CreateLabelSlice(vtkImageData* labels);

	void FilterRegions();

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
