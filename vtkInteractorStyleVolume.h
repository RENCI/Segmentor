#ifndef vtkInteractorStyleVolume_H
#define vtkInteractorStyleVolume_H

#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

class vtkCellPicker;

class SlicePipeline;
class VolumePipeline;

// Interaction flags
#define VTKIS_PAINT_VOLUME 2048

class vtkInteractorStyleVolume : public vtkInteractorStyleTrackballCamera {
public:
	static vtkInteractorStyleVolume* New();
	vtkTypeMacro(vtkInteractorStyleVolume, vtkInteractorStyleTrackballCamera);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnMouseMove() override;
	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnChar() override;

	virtual void Paint();
	virtual void StartPaint();
	virtual void EndPaint();

	void SetVolumePipeline(VolumePipeline* pipeline);
	void SetSlicePipeline(SlicePipeline* pipeline);

protected:
	vtkInteractorStyleVolume();
	~vtkInteractorStyleVolume() override;

	bool MouseMoved;

	vtkSmartPointer<vtkCellPicker> Picker;

	VolumePipeline* volumePipeline;
	SlicePipeline* slicePipeline;

private:
	vtkInteractorStyleVolume(const vtkInteractorStyleVolume&) = delete;
	void operator=(const vtkInteractorStyleVolume&) = delete;
};

#endif