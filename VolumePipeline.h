#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

class vtkImageData;
class vtkRenderer;
class vtkRenderWindowInteractor;

class VolumePipeline {
public:
  VolumePipeline(vtkRenderWindowInteractor* rwi);
  ~VolumePipeline();

  void SetInput(vtkImageData* input);

  vtkRenderer* GetRenderer();

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
