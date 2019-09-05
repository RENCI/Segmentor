#ifndef SlicePipeline_H
#define SlicePipeline_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkAlgorithmOutput;
class vtkContourFilter;
class vtkImageData;
class vtkImageSlice;
class vtkInteractorStyleSlice;
class vtkLookupTable;
class vtkObject;
class vtkPlane;
class vtkRenderer;
class vtkRenderWindowInteractor;

class SlicePipeline {
public:
	SlicePipeline(vtkRenderWindowInteractor* interactor, vtkLookupTable* lut);
	~SlicePipeline();

	void SetImageData(vtkImageData* data);
	void SetSegmentationData(vtkImageData* data);

	void SetShowProbe(bool show);
	void SetProbePosition(double x, double y, double z);

	void SetCurrentLabel(unsigned short label);
	
	void ToggleLabelSlice();
	void ToggleLabelOutlines();
	void ToggleRegionOutlines();

	void UpdateOverlay();
	void UpdatePlane();

	void Render();

	vtkSmartPointer<vtkRenderer> GetRenderer();
	vtkSmartPointer<vtkInteractorStyleSlice> GetInteractorStyle();

protected:
	// Data
	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkImageData> labels;

	// Rendering
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkRenderer> labelSliceRenderer;
	vtkSmartPointer<vtkRenderer> labelOutlinesRenderer;
	vtkSmartPointer<vtkRenderer> regionOutlinesRenderer;

	vtkSmartPointer<vtkInteractorStyleSlice> style;
	vtkSmartPointer<vtkLookupTable> labelColors;

	// Cut plane
	vtkSmartPointer<vtkPlane> plane;

	// Overlays
	vtkSmartPointer<vtkImageSlice> labelSlice;
	vtkSmartPointer<vtkActor> labelOutlines;
	vtkSmartPointer<vtkActor> regionOutlines;

	// Probe
	vtkSmartPointer<vtkActor> probe;
	void CreateProbe();
	void UpdateProbe(vtkImageData* data);

	// Slices
	void CreateDataSlice(vtkImageData* data);
	void CreateLabelSlice(vtkImageData* labels);

	static void cameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
};

#endif
