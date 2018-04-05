#ifndef VolumePipeline_H
#define VolumePipeline_H

#include <vtkSmartPointer.h>

class vtkRenderer;
class vtkRenderWindowInteractor;

class VolumePipeline {
public:

  VolumePipeline(vtkRenderWindowInteractor* rwi);
  ~VolumePipeline();

protected:

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
