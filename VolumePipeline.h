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

class VolumePipeline {
public:
  VolumePipeline(vtkRenderWindowInteractor* rwi);
  ~VolumePipeline();

  void SetSegmentationData(vtkImageData* data);

  void SetProbeVisiblity(bool visibility);
  void SetProbePosition(double x, double y, double z);

  void SetSmoothSurfaces(bool smooth);
  void ToggleSmoothSurfaces();

  void SetLabel(unsigned short label);
  void SetThresholdLabels(bool doThreshold);
  void ToggleThresholdLabels();

  void Render();

  vtkRenderer* GetRenderer();	
  vtkInteractorStyleVolume* GetInteractorStyle();

protected:
	bool thresholdLabels;
	bool smoothSurfaces;
	unsigned short currentLabel;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleVolume> style;
	vtkSmartPointer<vtkLookupTable> labelColors;

	// Regions
	std::vector<vtkSmartPointer<vtkActor>> regionActors;
	void ExtractRegions(vtkImageData* labels);

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);
};

#endif
