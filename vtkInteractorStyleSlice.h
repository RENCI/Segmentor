#ifndef vtkInteractorStyleSlice_H
#define vtkInteractorStyleSlice_H

#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

class vtkCellPicker;
class SlicePipeline;
class VolumePipeline;

class vtkInteractorStyleSlice : public vtkInteractorStyleImage {
public:
	static vtkInteractorStyleSlice* New();
	vtkTypeMacro(vtkInteractorStyleSlice, vtkInteractorStyleImage);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnMouseMove() override;

	vtkSetObjectMacro(Labels, vtkImageData);
	vtkGetObjectMacro(Labels, vtkImageData);

	void SetVolumePipeline(VolumePipeline* pipeline);
	void SetSlicePipeline(SlicePipeline* pipeline);

protected:
	vtkInteractorStyleSlice();
	~vtkInteractorStyleSlice() override;

	bool MouseMoved;

	vtkSmartPointer<vtkCellPicker> Picker;
	
	vtkImageData* Labels;

	VolumePipeline* volumePipeline;
	SlicePipeline* slicePipeline;

private:
	vtkInteractorStyleSlice(const vtkInteractorStyleSlice&) = delete;
	void operator=(const vtkInteractorStyleSlice&) = delete;
};

#endif