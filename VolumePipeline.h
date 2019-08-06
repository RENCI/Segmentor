#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkImageData;
class vtkLookupTable;
class vtkRenderer;
class vtkRenderWindowInteractor;

class vtkInteractorStyleVolume;

class Region;
class RegionSurface;

class VolumePipeline {
public:
  VolumePipeline(vtkRenderWindowInteractor* rwi, vtkLookupTable* lut);
  ~VolumePipeline();

  void SetRegions(vtkImageData* data, std::vector<Region*> regions);

  void SetLabel(unsigned short label);

  void SetProbeVisiblity(bool visibility);
  void SetProbePosition(double x, double y, double z);

  void SetSmoothSurfaces(bool smooth);
  void ToggleSmoothSurfaces();

  void SetSmoothShading(bool smooth);
  void ToggleSmoothShading();

  void SetFilterLabels(bool filter);
  void ToggleFilterLabels();

  void Render();

  vtkRenderer* GetRenderer();	
  vtkInteractorStyleVolume* GetInteractorStyle();

protected:
	bool filterLabels;
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

	void FilterLabels();
};

#endif
