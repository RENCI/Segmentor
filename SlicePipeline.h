#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class vtkContourFilter;
class vtkImageData;
class vtkImageSlice;
class vtkRenderer;
class vtkRenderWindowInteractor;

class SlicePipeline {
public:
	SlicePipeline(vtkRenderWindowInteractor* rwi);
	~SlicePipeline();

	void SetInput(vtkImageData* data, vtkImageData* labels);

	vtkRenderer* GetRenderer();

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;

	// Slices
	vtkSmartPointer<vtkImageSlice> CreateDataSlice(vtkImageData* data);
	vtkSmartPointer<vtkImageSlice> CreateLabelSlice(vtkImageData* labels);
};

#endif
