#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkDiscreteFlyingEdges3D;
class vtkImageData;
class vtkInteractorStyleVolume;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkVolume;

class VolumePipeline {
public:
  VolumePipeline(vtkRenderWindowInteractor* rwi);
  ~VolumePipeline();

  void SetSegmentationData(vtkImageData* labels);

  void ShowProbe(bool show = true);
  void SetProbePosition(double x, double y, double z);

  void Render();

  vtkRenderer* GetRenderer();	
  vtkInteractorStyleVolume* GetInteractorStyle();
  vtkAlgorithmOutput* GetContour();

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleVolume> style;

	// Contour
	vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	vtkSmartPointer<vtkActor> CreateGeometry(vtkImageData* data);
	vtkSmartPointer<vtkVolume> CreateVolume(vtkImageData* data);
};

#endif
