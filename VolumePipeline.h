#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkBox;
class vtkImageData;
class vtkLookupTable;
class vtkPlaneSource;
class vtkObject;
class vtkOutlineCornerFilter;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkTextActor;

class vtkInteractorStyleVolume;

class Region;
class RegionSurface;

class VolumePipeline {
public:
  VolumePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut);
  ~VolumePipeline();

  void SetRegions(vtkImageData* data, std::vector<Region*> regions);

  void AddSurface(Region* region);
  void RemoveSurface(unsigned short label);
  void SetSurfaceDone(unsigned short label, bool done);

  void SetShowProbe(bool visibility);
  void SetProbePosition(double x, double y, double z);

  void SetInteractionMode(enum InteractionMode mode);

  void SetCurrentLabel(unsigned short label);

  void SetSmoothSurfaces(bool smooth);
  void ToggleSmoothSurfaces();

  void SetSmoothShading(bool smooth);
  void ToggleSmoothShading();

  void SetFilterLabel(bool filter);
  void ToggleFilterLabel();

  void SetFilterPlane(bool filter);
  void ToggleFilterPlane();

  void SetShowPlane(bool show);
  void ToggleShowPlane();
  void UpdatePlane();

  void Render();

  vtkRenderer* GetRenderer();	
  vtkInteractorStyleVolume* GetInteractorStyle();

protected:
	bool filterLabel;
	bool filterPlane;
	bool smoothSurfaces;
	bool smoothShading;
	unsigned short currentLabel;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleVolume> style;
	vtkSmartPointer<vtkLookupTable> labelColors;

	// Region surfaces
	std::vector<RegionSurface*> surfaces;
	void RemoveSurfaces();

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

	void FilterLabels();

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
