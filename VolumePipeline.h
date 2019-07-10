#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkDiscreteFlyingEdges3D;
class vtkImageData;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkVolume;

class VolumePipeline {
public:
  VolumePipeline(vtkRenderWindowInteractor* rwi);
  ~VolumePipeline();

  void SetInput(vtkImageData* input, vtkImageData* labels);

  vtkRenderer* GetRenderer();
  vtkAlgorithmOutput* GetContour();

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;

	// Contour
	vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour;

	vtkSmartPointer<vtkActor> CreateGeometry(vtkImageData* data);
	vtkSmartPointer<vtkVolume> CreateVolume(vtkImageData* data);
};

#endif
