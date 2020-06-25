#ifndef vtkInteractorStyleVolume_H
#define vtkInteractorStyleVolume_H

#include <vtkCommand.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

#include "InteractionEnums.h"

class vtkCellPicker;

// Interaction flags
#define VTKIS_PAINT_VOLUME 2048
#define VTKIS_ERASE_VOLUME 2049
#define VTKIS_SELECT_VOLUME 2050
#define VTKIS_SLICE_VOLUME 2051

class vtkInteractorStyleVolume : public vtkInteractorStyleTrackballCamera {
public:
	static vtkInteractorStyleVolume* New();
	vtkTypeMacro(vtkInteractorStyleVolume, vtkInteractorStyleTrackballCamera);
	void PrintSelf(ostream& os, vtkIndent indent) override;

	void SetMode(enum InteractionMode mode);

	void FlyTo(double focalPoint[3]);

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
	virtual void StartErase();
	virtual void EndErase();
	virtual void StartSlice();
	virtual void EndSlice();
	virtual void Slice();

	enum VolumeEventIds {
		SelectLabelEvent = vtkCommand::UserEvent + 1,
		StartPaintEvent,
		PaintEvent,
		EndPaintEvent, 
		StartEraseEvent,
		EraseEvent,
		EndEraseEvent
	};

protected:
	vtkInteractorStyleVolume();
	~vtkInteractorStyleVolume() override;

	bool MouseMoved;

	enum InteractionMode Mode;

	vtkSmartPointer<vtkCellPicker> Picker;

	void SetOrientation(const double leftToRight[3], const double viewUp[3]);

private:
	vtkInteractorStyleVolume(const vtkInteractorStyleVolume&) = delete;
	void operator=(const vtkInteractorStyleVolume&) = delete;
};

#endif