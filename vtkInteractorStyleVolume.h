#ifndef vtkInteractorStyleVolume_H
#define vtkInteractorStyleVolume_H

#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

class vtkCellPicker;

class SlicePipeline;
class VolumePipeline;

class vtkInteractorStyleVolume : public vtkInteractorStyleTrackballCamera {
public:
	static vtkInteractorStyleVolume* New();
	vtkTypeMacro(vtkInteractorStyleVolume, vtkInteractorStyleTrackballCamera);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnMouseMove() override;

	void SetVolumePipeline(VolumePipeline* pipeline);
	void SetSlicePipeline(SlicePipeline* pipeline);

protected:
	vtkInteractorStyleVolume();
	~vtkInteractorStyleVolume() override;

	vtkSmartPointer<vtkCellPicker> Picker;

	VolumePipeline* volumePipeline;
	SlicePipeline* slicePipeline;

private:
	vtkInteractorStyleVolume(const vtkInteractorStyleVolume&) = delete;
	void operator=(const vtkInteractorStyleVolume&) = delete;
};

#endif