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

	void SetProbeVisiblity(bool visibility);
	void SetProbePosition(double x, double y, double z);

	void PickLabel(int x, int y, int z);
	void Paint(int x, int y, int z);

	void PickPointLabel(double x, double y, double z);
	void PaintPoint(double x, double y, double z);

	unsigned short GetLabel();
	void SetLabel(unsigned int newLabel);

	void Render();

	vtkRenderer* GetRenderer();
	vtkInteractorStyleSlice* GetInteractorStyle();
	vtkPlane* GetPlane();

protected:
	// Data
	vtkSmartPointer<vtkImageData> labels;
	unsigned short label;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkInteractorStyleSlice> style;

	// Plane for slicing
	vtkSmartPointer<vtkPlane> plane;

	// Slices
	vtkSmartPointer<vtkImageSlice> labelSlice;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	// Slices
	vtkSmartPointer<vtkImageSlice> CreateDataSlice(vtkImageData* data);
	void CreateLabelSlice(vtkImageData* labels);
	vtkSmartPointer<vtkActor> CreateLabelSlice2(vtkImageData* labels);
};

#endif
