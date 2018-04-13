#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

class vtkAlgorithm;
class vtkRenderer;
class vtkRenderWindowInteractor;

class VolumePipeline {
public:

  VolumePipeline(vtkRenderWindowInteractor* rwi);
  ~VolumePipeline();

  void SetInput(vtkAlgorithm* input);

protected:

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
