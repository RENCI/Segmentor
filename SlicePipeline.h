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
	void SetSegmentationData(vtkImageData* data);

	void SetShowProbe(bool show);
	void SetProbePosition(double x, double y, double z);

	void SetCurrentLabel(unsigned short label);

	void Render();

	vtkSmartPointer<vtkRenderer> GetRenderer();
	vtkSmartPointer<vtkInteractorStyleSlice> GetInteractorStyle();

protected:
	// Data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleSlice> style;

	// Slices
	vtkSmartPointer<vtkImageSlice> labelSlice;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	// Slices
	vtkSmartPointer<vtkImageSlice> CreateDataSlice(vtkImageData* data);
	void CreateLabelSlice(vtkImageData* labels);
};

#endif
