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

	void SetInput(vtkImageData* data, vtkImageData* labels);

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
