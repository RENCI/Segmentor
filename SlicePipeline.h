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

	void ShowProbe(bool show = true);
	void SetProbePosition(double x, double y, double z);

	void Render();

	vtkRenderer* GetRenderer();
	vtkInteractorStyleSlice* GetInteractorStyle();
	vtkPlane* GetPlane();

protected:
	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleSlice> style;

	// Plane for slicing
	vtkSmartPointer<vtkPlane> plane;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	// Slices
	vtkSmartPointer<vtkImageSlice> CreateDataSlice(vtkImageData* data);
	vtkSmartPointer<vtkImageSlice> CreateLabelSlice(vtkImageData* labels);
	vtkSmartPointer<vtkActor> CreateLabelSlice2(vtkImageData* labels);
};

#endif
