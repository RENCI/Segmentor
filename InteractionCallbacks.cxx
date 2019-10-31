#include "InteractionCallbacks.h"

#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkInteractorStyle.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#include "VisualizationContainer.h"
#include "VolumePipeline.h"
#include "SlicePipeline.h"

bool InteractionCallbacks::firstCameraCallback = true;

vtkSmartPointer<vtkCellPicker> InteractionCallbacks::picker = vtkSmartPointer<vtkCellPicker>::New();

InteractionCallbacks::InteractionCallbacks() {
}

InteractionCallbacks::~InteractionCallbacks() {
}

void InteractionCallbacks::VolumeCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	if (!firstCameraCallback) return;

	firstCameraCallback = false;

	vtkCamera* volumeCamera = static_cast<vtkCamera*>(caller);

	vtkRenderer* sliceRenderer = static_cast<vtkRenderer*>(clientData);
	vtkCamera* sliceCamera = sliceRenderer->GetActiveCamera();

	sliceCamera->SetFocalPoint(volumeCamera->GetFocalPoint());
	sliceCamera->SetPosition(volumeCamera->GetPosition());
	sliceCamera->SetViewUp(volumeCamera->GetViewUp());

	sliceCamera->SetClippingRange(volumeCamera->GetDistance() - 0.5, volumeCamera->GetDistance() + 0.5);

	sliceRenderer->GetRenderWindow()->Render();

	firstCameraCallback = true;
}

void InteractionCallbacks::SliceCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	if (!firstCameraCallback) return;

	firstCameraCallback = false;

	vtkCamera* sliceCamera = static_cast<vtkCamera*>(caller);

	vtkRenderer* volumeRenderer = static_cast<vtkRenderer*>(clientData);
	vtkCamera* volumeCamera = volumeRenderer->GetActiveCamera();

	volumeCamera->SetFocalPoint(sliceCamera->GetFocalPoint());
	volumeCamera->SetPosition(sliceCamera->GetPosition());
	volumeCamera->SetViewUp(sliceCamera->GetViewUp());

	volumeRenderer->ResetCameraClippingRange();
	volumeRenderer->GetRenderWindow()->Render();

	firstCameraCallback = true;
}

void InteractionCallbacks::OnChar(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkRenderWindowInteractor*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	switch (rwi->GetKeyCode()) {
	case 32:
		vis->ToggleInteractionMode();
		break;

	case 's':
	case 'S':
		vis->GetVolumePipeline()->ToggleSmoothSurfaces();
		break;

	case 'n':
	case 'N':
		vis->GetVolumePipeline()->ToggleSmoothShading();
		break;
	
	case 'p':
	case 'P':
		vis->GetVolumePipeline()->ToggleFilterPlane();
		break;

	case 'o':
	case 'O':
		vis->GetVolumePipeline()->ToggleShowPlane();
		break;

	case 'l':
	case 'L':
		vis->GetVolumePipeline()->ToggleFilterLabel();
		break;

	case 'g':
	case 'G': {
			// Pick at the mouse location provided by the interactor	
			int pick = Pick(rwi);

			if (pick) {
				// Get the pixel indeces for the pick event		
				int* p = SlicePick();

				// XXX: HACK TO GUARD AGAINST INVALID VOLUME PICK
				//		NEED TO FIX VOLUME PICKING AND DIFFERENTIATE BETWEEN THEM FOR KEYSTROKE CALLBACK

				if (p[0] == 0 && p[1] == 0 && p[3] == 0) break;
				
				vis->GrowRegion(p[0], p[1], p[2]);
				vis->Render();
			}
			break;
		}

	case '1':
		vis->GetSlicePipeline()->ToggleLabelSlice();
		break;

	case '2':
		vis->GetSlicePipeline()->ToggleLabelOutlines();
		break;

	case '3':
		vis->GetSlicePipeline()->ToggleRegionOutlines();
		break;
	}
}

void InteractionCallbacks::VolumeSelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double* p = VolumePick();
		vis->PickPointLabel(p[0], p[1], p[2]);
		vis->Render();
	}
}

void InteractionCallbacks::SliceSelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the pixel indeces for the pick event		
		int* p = SlicePick();
		vis->PickLabel(p[0], p[1], p[2]);
		vis->Render();
	}
}

void InteractionCallbacks::VolumePaint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double* p = VolumePick();
		vis->PaintPoint(p[0], p[1], p[2]);
		vis->Render();
	}
}

void InteractionCallbacks::SlicePaint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the pixel indeces for the pick event
		int* p = SlicePick();
		vis->Paint(p[0], p[1], p[2]);
		vis->Render();		
	}
}

void InteractionCallbacks::VolumeErase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double* p = VolumePick();
		vis->ErasePoint(p[0], p[1], p[2]);
		vis->Render();
	}
}

void InteractionCallbacks::SliceErase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the pixel indeces for the pick event
		int* p = SlicePick();
		vis->Erase(p[0], p[1], p[2]);
		vis->Render();
	}
}

void InteractionCallbacks::VolumeMouseMove(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	MouseMove(static_cast<vtkRenderWindowInteractor*>(caller), static_cast<VisualizationContainer*>(clientData), VolumeView);
}

void InteractionCallbacks::SliceMouseMove(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	MouseMove(static_cast<vtkRenderWindowInteractor*>(caller), static_cast<VisualizationContainer*>(clientData), SliceView);
}

void InteractionCallbacks::MouseMove(vtkRenderWindowInteractor* rwi, VisualizationContainer* vis, ViewType viewType) {
	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the point coordinate for the pick event
		if (viewType == SliceView) {
			int* p = SlicePick();
			vis->GetVolumePipeline()->SetProbePosition(p[0], p[1], p[2]);
			vis->GetSlicePipeline()->SetProbePosition(p[0], p[1], p[2]);
		}
		else {
			double* p = VolumePick(); 
			vis->GetVolumePipeline()->SetProbePosition(p[0], p[1], p[2]);
			vis->GetSlicePipeline()->SetProbePosition(p[0], p[1], p[2]);		
		}

		// Update probes
		vis->GetVolumePipeline()->SetShowProbe(true);
		vis->GetSlicePipeline()->SetShowProbe(true);
	}
	else {
		vis->GetVolumePipeline()->SetShowProbe(false);
		vis->GetSlicePipeline()->SetShowProbe(false);
	}

	vis->Render();
}

int InteractionCallbacks::Pick(vtkRenderWindowInteractor* rwi) {
	int x = rwi->GetEventPosition()[0];
	int y = rwi->GetEventPosition()[1];

	return picker->Pick(x, y, 0.0, rwi->FindPokedRenderer(x, y));
}

double* InteractionCallbacks::VolumePick() {
	return picker->GetPickPosition();
}

int* InteractionCallbacks::SlicePick() {
	return picker->GetPointIJK();
}