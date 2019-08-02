#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkDiscreteFlyingEdges3D;
class vtkImageData;
class vtkImageThreshold;
class vtkInteractorStyleVolume;
class vtkPolyDataMapper;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkVolume;
class vtkWindowedSincPolyDataFilter;

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
  void SetThresholdLabels(bool threshold);
  void ToggleThresholdLabels();

  void Render();

  vtkRenderer* GetRenderer();	
  vtkInteractorStyleVolume* GetInteractorStyle();
  vtkAlgorithmOutput* GetContour();

protected:
	bool thresholdLabels;
	bool smoothSurfaces;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleVolume> style;

	// Pipeline
	vtkSmartPointer<vtkImageThreshold> threshold;
	vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour;
	vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother;
	vtkSmartPointer<vtkPolyDataMapper> mapper;
	vtkSmartPointer<vtkActor> actor;

	void CreatePipeline();
	void UpdatePipeline();

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);
};

#endif
