#ifndef vtkInteractorStyleSlice_H
#define vtkInteractorStyleSlice_H

#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

#include "InteractionEnums.h"

class vtkCellPicker;

// Interaction flags
#define VTKIS_PAINT_SLICE 2048
#define VTKIS_OVERWRITE_SLICE 2049
#define VTKIS_ERASE_SLICE 2050
#define VTKIS_SELECT_SLICE 2051

class vtkInteractorStyleSlice : public vtkInteractorStyleImage {
public:
	static vtkInteractorStyleSlice* New();
	vtkTypeMacro(vtkInteractorStyleSlice, vtkInteractorStyleImage);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void SetMode(enum InteractionMode mode);

	void OnMouseMove() override;
	void OnLeftButtonDown() override;
	void OnLeftButtonUp() override;
	void OnMiddleButtonDown() override;
	void OnMiddleButtonUp() override;
	void OnRightButtonDown() override;
	void OnRightButtonUp() override;
	void OnChar() override;

	virtual void StartSelect();
	virtual void EndSelect();
	virtual void StartPaint();
	virtual void EndPaint();
	virtual void StartOverwrite();
	virtual void EndOverwrite();
	virtual void StartErase();
	virtual void EndErase();

	virtual void WindowLevel() override;
	double GetWindow();
	double GetLevel();

	enum SliceEventIds {
		SelectLabelEvent = vtkCommand::UserEvent + 1,
		StartPaintEvent,
		PaintEvent,
		EndPaintEvent,
		StartOverwriteEvent,
		OverwriteEvent,
		EndOverwriteEvent,
		StartEraseEvent,
		EraseEvent,
		EndEraseEvent
	};

protected:
	vtkInteractorStyleSlice();
	~vtkInteractorStyleSlice() override;

	bool MouseMoved;

	enum InteractionMode Mode;

	vtkSmartPointer<vtkCellPicker> Picker;

	void SetOrientation(const double leftToRight[3], const double viewUp[3]);

private:
	vtkInteractorStyleSlice(const vtkInteractorStyleSlice&) = delete;
	void operator=(const vtkInteractorStyleSlice&) = delete;
};

#endif
