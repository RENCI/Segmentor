#ifndef VolumeView_H
#define VolumeView_H

#include <vtkSmartPointer.h>

#include <vector>

#include "InteractionEnums.h"

class vtkActor;
class vtkBox;
class vtkColorTransferFunction;
class vtkCubeAxesActor;
class vtkImageCast;
class vtkImageData;
class vtkLookupTable;
class vtkImageMapToColors;
class vtkImageMask;
class vtkPiecewiseFunction;
class vtkPlaneSource;
class vtkObject;
class vtkOutlineCornerFilter;
class vtkPassThrough;
class vtkRenderer;
class vtkRenderWindowInteractor; 
class vtkSmartVolumeMapper;
class vtkTextActor;
class vtkVolume;

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

	void SetImageData(vtkImageData* imageData);
	void SetRegions(vtkImageData* imageLabels, RegionCollection* newRegions);
	void AddRegion(Region* region);

	void Reset();
	void Enable(bool enable);

	void UpdateVoxelSize(vtkImageData* data);

	void SetShowProbe(bool visibility);
	void SetProbePosition(double x, double y, double z);

	void SetInteractionMode(enum InteractionMode mode);

	void SetCurrentRegion(Region* region);

	void HighlightRegion(Region* region);

	void ShowRegion(Region* region, bool show = true);

	bool GetSmoothSurfaces();
	void SetSmoothSurfaces(bool smooth);
	void ToggleSmoothSurfaces();

	bool GetSmoothShading();
	void SetSmoothShading(bool smooth);
	void ToggleSmoothShading();

	bool GetVolumeRendering();
	void SetVolumeRendering(bool useVolumeRendering);
	void ToggleVolumeRendering();

	bool GetShowPlane();
	void SetShowPlane(bool show);
	void ToggleShowPlane();
	void UpdatePlane();

	double GetVisibleOpacity();
	void SetVisibleOpacity(double opacity, bool apply);
	void UpdateVisibleOpacity(bool apply);

	void SetBrushRadius(int radius);

	// Volume rendering
	void SetWindowLevel(double window, double level);
	void UpdateVolumeMask(bool filter);
	void SetVolumeRenderingGradientOpacity(bool gradientOpacity);
	bool GetVolumeRenderingGradientOpacity();
	void SetVolumeRenderingAutoAdjustSampling(bool autoAdjust);
	bool GetVolumeRenderingAutoAdjustSampling();

	void Render();

	vtkRenderer* GetRenderer();	
	vtkInteractorStyleVolume* GetInteractorStyle();

protected:
	// Data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	bool smoothSurfaces;
	bool smoothShading;
	bool volumeRendering;
	
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

	// Volume rendering
	vtkSmartPointer<vtkImageCast> volumeCopy;
	vtkSmartPointer<vtkImageMask> volumeMask;
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper;
	vtkSmartPointer<vtkVolume> volume;
	void CreateVolumeRenderer();
	void UpdateVolumeRenderer();
	void UpdateVolumeRenderingTransferFunctions(double x1, double x2);
	
	double visibleOpacity;

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
