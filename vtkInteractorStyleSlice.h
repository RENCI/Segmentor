#ifndef vtkInteractorStyleSlice_H
#define vtkInteractorStyleSlice_H

#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

class vtkCellPicker;

// Interaction flags
#define VTKIS_PAINT_SLICE 2048
#define VTKIS_ERASE_SLICE 2049

class vtkInteractorStyleSlice : public vtkInteractorStyleImage {
public:
	static vtkInteractorStyleSlice* New();
	vtkTypeMacro(vtkInteractorStyleSlice, vtkInteractorStyleImage);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void OnMouseMove() override;
	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnRightButtonDown() override;
	void OnRightButtonUp() override;
	void OnChar() override;

	virtual void StartPaint();
	virtual void EndPaint();
	virtual void StartErase();
	virtual void EndErase();

	enum SliceEventIds {
		SelectLabelEvent = vtkCommand::UserEvent + 1,
		StartPaintEvent,
		PaintEvent,
		EndPaintEvent,
		StartEraseEvent,
		EraseEvent,
		EndEraseEvent
	};

protected:
	vtkInteractorStyleSlice();
	~vtkInteractorStyleSlice() override;

	bool MouseMoved;

	vtkSmartPointer<vtkCellPicker> Picker;

private:
	vtkInteractorStyleSlice(const vtkInteractorStyleSlice&) = delete;
	void operator=(const vtkInteractorStyleSlice&) = delete;
};

#endif