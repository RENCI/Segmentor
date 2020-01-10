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
	
	static void OnChar(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void VolumeSelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void SliceSelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void VolumePaint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void SlicePaint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void VolumeErase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void SliceErase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

	static void VolumeMouseMove(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);
	static void SliceMouseMove(vtkObject* caller, unsigned long eventId, void* clientData, void *callData);

private:
	static bool firstCameraCallback;

	static vtkSmartPointer<vtkCellPicker> picker;

	enum ViewType {
		VolumeView,
		SliceView
	};

	static void MouseMove(vtkRenderWindowInteractor* rwi, VisualizationContainer* vis, ViewType viewType);

	static int Pick(vtkRenderWindowInteractor* rwi);
	static void VolumePick(int p[3]);
	static void SlicePick(int p[3]);

	InteractionCallbacks();
	~InteractionCallbacks();
};

#endif