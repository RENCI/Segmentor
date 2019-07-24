#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkContourFilter;
class vtkImageData;
class vtkImageSlice;
class vtkInteractorStyleSlice;
class vtkPlane;
class vtkRenderer;
class vtkRenderWindowInteractor;

class SlicePipeline {
public:
	SlicePipeline(vtkRenderWindowInteractor* rwi);
	~SlicePipeline();

	void SetImageData(vtkImageData* data);
	void SetSegmentationData(vtkImageData* labels);

	vtkRenderer* GetRenderer();
	vtkPlane* GetPlane();

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleSlice> style;

	// Plane for slicing
	vtkSmartPointer<vtkPlane> plane;

	// Slices
	vtkSmartPointer<vtkImageSlice> CreateDataSlice(vtkImageData* data);
	vtkSmartPointer<vtkImageSlice> CreateLabelSlice(vtkImageData* labels);
	vtkSmartPointer<vtkActor> CreateLabelSlice2(vtkImageData* labels);
};

#endif
