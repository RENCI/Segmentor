#ifndef InteractionCallbacks_H
#define InteractionCallbacks_H

#include "vtkSmartPointer.h"

class vtkObject;
class vtkCellPicker;
class vtkRenderWindowInteractor;

class VisualizationContainer;

class InteractionCallbacks {
public:
	static void VolumeCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void SliceCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void CameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	
	static void OnChar(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void SelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void Paint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void Erase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void MouseMove(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void WindowLevel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

private:
	static bool firstCameraCallback;

	static vtkSmartPointer<vtkCellPicker> picker;

	enum ViewType {
		VolumeView,
		SliceView
	};
	
	static int Pick(vtkRenderWindowInteractor* rwi);
	static void PickPosition(double p[3]);

	InteractionCallbacks();
	~InteractionCallbacks();
};

#endif