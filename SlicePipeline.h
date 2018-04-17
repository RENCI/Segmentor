#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class vtkImageData;
class vtkRenderer;
class vtkRenderWindowInteractor;

class SlicePipeline {
public:

	SlicePipeline(vtkRenderWindowInteractor* rwi);
	~SlicePipeline();

	void SetInput(vtkImageData* input);

protected:

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
