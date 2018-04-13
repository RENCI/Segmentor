#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class vtkAlgorithm;
class vtkRenderer;
class vtkRenderWindowInteractor;

class SlicePipeline {
public:

	SlicePipeline(vtkRenderWindowInteractor* rwi);
	~SlicePipeline();

	void SetInput(vtkAlgorithm* input);

protected:

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
};

#endif
