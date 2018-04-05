#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkRenderWindowInteractor;

class SlicePipeline {
public:

  SlicePipeline(vtkRenderWindowInteractor* rwi);
  ~SlicePipeline();

protected:

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
