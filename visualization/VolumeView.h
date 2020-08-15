#ifndef VolumeView_H
#define VolumeView_H

#include <vtkSmartPointer.h>

#include <vector>

#include "InteractionEnums.h"

class vtkActor;
class vtkBox;
class vtkCubeAxesActor;
class vtkImageData;
class vtkPlaneSource;
class vtkObject;
class vtkOutlineCornerFilter;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkTextActor;

class vtkInteractorStyleVolume;

class Brush;
class Probe;
class Region;
class RegionSurface;
class RegionCollection;

class VolumeView {
public:
	VolumeView(vtkRenderWindowInteractor* interactor);
	~VolumeView();

	void SetImageData(vtkImageData* data);
	void SetRegions(vtkImageData* data, RegionCollection* newRegions);
	void AddRegion(Region* region);

	void Reset();
	void Enable(bool enable);

	void UpdateVoxelSize(vtkImageData* data);

	void SetShowProbe(bool visibility);
	void SetProbePosition(double x, double y, double z);

	void SetInteractionMode(enum InteractionMode mode);
	void SetFilterMode(enum FilterMode mode);

	void SetCurrentRegion(Region* region);

	void HighlightRegion(Region* region);

	void ShowRegion(Region* region, bool show = true);

	bool GetSmoothSurfaces();
	void SetSmoothSurfaces(bool smooth);
	void ToggleSmoothSurfaces();

	bool GetSmoothShading();
	void SetSmoothShading(bool smooth);
	void ToggleSmoothShading();

	bool GetShowPlane();
	void SetShowPlane(bool show);
	void ToggleShowPlane();
	void UpdatePlane();

	void SetNeighborOpacity(double opacity);

	void SetBrushRadius(int radius);

	void Render();

	vtkRenderer* GetRenderer();	
	vtkInteractorStyleVolume* GetInteractorStyle();

protected:
	FilterMode filterMode;
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
	Probe* probe;

	// Brush
	Brush* brush;

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

	// Axes
	vtkSmartPointer<vtkCubeAxesActor> axes;
	void CreateAxes();
	void UpdateAxes(vtkImageData* data);

	// Interaction mode label
	vtkSmartPointer<vtkTextActor> interactionModeLabel;
	void CreateInteractionModeLabel();

	void FilterRegions();

	double neighborOpacity;

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
