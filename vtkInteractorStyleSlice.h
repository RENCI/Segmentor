#ifndef vtkInteractorStyleSlice_H
#define vtkInteractorStyleSlice_H

#include <vtkInteractorStyleImage.h>

#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

#include <vtkPropPicker.h>

class vtkInteractorStyleSlice : public vtkInteractorStyleImage {
public:
	static vtkInteractorStyleSlice* New();
	vtkTypeMacro(vtkInteractorStyleSlice, vtkInteractorStyleImage);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnMouseMove() override;

	vtkGetObjectMacro(Picker, vtkPropPicker);

protected:
	vtkInteractorStyleSlice();
	~vtkInteractorStyleSlice() override;

	bool MouseMoved;

	vtkPropPicker* Picker;

private:
	vtkInteractorStyleSlice(const vtkInteractorStyleSlice&) = delete;
	void operator=(const vtkInteractorStyleSlice&) = delete;
};

#endif