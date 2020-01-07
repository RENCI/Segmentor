#ifndef VolumeView_H
#define VolumeView_H

#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkBox;
class vtkImageData;
class vtkPlaneSource;
class vtkObject;
class vtkOutlineCornerFilter;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkTextActor;

class vtkInteractorStyleVolume;

class Region;
class RegionSurface;
class RegionCollection;

enum InteractionMode;

class VolumeView {
public:
	VolumeView(vtkRenderWindowInteractor* interactor);
	~VolumeView();

	void SetRegions(vtkImageData* data, RegionCollection* newRegions);
	void AddRegion(Region* region);

	void Reset();

	void SetShowProbe(bool visibility);
	void SetProbePosition(double x, double y, double z);

	void SetInteractionMode(InteractionMode mode);

	void SetCurrentRegion(Region* region);

	void HighlightRegion(Region* region);

	bool GetSmoothSurfaces();
	void SetSmoothSurfaces(bool smooth);
	void ToggleSmoothSurfaces();

	bool GetSmoothShading();
	void SetSmoothShading(bool smooth);
	void ToggleSmoothShading();

	bool GetFilterRegion();
	void SetFilterRegion(bool filter);
	void ToggleFilterRegion();

	bool GetFilterPlane();
	void SetFilterPlane(bool filter);
	void ToggleFilterPlane();

	bool GetShowPlane();
	void SetShowPlane(bool show);
	void ToggleShowPlane();
	void UpdatePlane();

	void Render();

	vtkRenderer* GetRenderer();	
	vtkInteractorStyleVolume* GetInteractorStyle();

protected:
	bool filterRegion;
	bool filterPlane;
	bool smoothSurfaces;
	bool smoothShading;
	
	Region* currentRegion;
	Region* highlightRegion;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleVolume> style;

	// Regions
	RegionCollection* regions;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	// Plane	
	vtkSmartPointer<vtkPlaneSource> planeSource;
	vtkSmartPointer<vtkActor> plane;
	vtkSmartPointer<vtkActor> planeWire;
	void CreatePlane();
	void UpdatePlane(vtkImageData* data);

	// Volume corners
	vtkSmartPointer<vtkOutlineCornerFilter> cornerFilter;
	vtkSmartPointer<vtkActor> corners;
	void CreateCorners();
	void UpdateCorners(vtkImageData* data);

	// Interaction mode label
	vtkSmartPointer<vtkTextActor> interactionModeLabel;
	void CreateInteractionModeLabel();

	void FilterRegions();

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
