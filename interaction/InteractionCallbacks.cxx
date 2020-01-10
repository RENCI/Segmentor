#include "InteractionCallbacks.h"

#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkInteractorStyle.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#include "VisualizationContainer.h"
#include "VolumeView.h"
#include "SliceView.h"

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
	case 'g':
	case 'G': {
		// Pick at the mouse location provided by the interactor	
		int pick = Pick(rwi);

		if (pick) {
			// Get the pixel indeces for the pick event		
			int p[3];
			SlicePick(p);

			// XXX: HACK TO GUARD AGAINST INVALID VOLUME PICK
			//		NEED TO FIX VOLUME PICKING AND DIFFERENTIATE BETWEEN THEM FOR KEYSTROKE CALLBACK

			if (p[0] == 0 && p[1] == 0 && p[3] == 0) break;
				
			vis->GrowCurrentRegion(p[0], p[1], p[2]);
		}
		break;
	}

	case 'u':
	case 'U':
		vis->RelabelCurrentRegion();
		break;

	case 'm':
	case 'M': {
		// Pick at the mouse location provided by the interactor	
		int pick = Pick(rwi);

		if (pick) {
			// Get the pixel indeces for the pick event		
			int p[3];
			SlicePick(p);

			// XXX: HACK TO GUARD AGAINST INVALID VOLUME PICK
			//		NEED TO FIX VOLUME PICKING AND DIFFERENTIATE BETWEEN THEM FOR KEYSTROKE CALLBACK

			if (p[0] == 0 && p[1] == 0 && p[3] == 0) break;

			vis->MergeWithCurrentRegion(p[0], p[1], p[2]);
		}
		break;
	}

	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		vis->SplitCurrentRegion(rwi->GetKeyCode() - '0');
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
		//double p[3];
		//VolumePick(p);
		//vis->PickPointLabel(p[0], p[1], p[2]);
		int p[3];
		VolumePick(p);
		vis->PickLabel(p[0], p[1], p[2]);
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
		int p[3];
		SlicePick(p);
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
		int p[3];
		VolumePick(p);
		vis->Paint(p[0], p[1], p[2]);
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
		int p[3];
		SlicePick(p);
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
		int p[3];
		VolumePick(p);
		vis->Erase(p[0], p[1], p[2]);
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
		int p[3];
		SlicePick(p);
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
			int p[3];
			SlicePick(p);
			vis->GetVolumeView()->SetProbePosition(p[0], p[1], p[2]);
			vis->GetSliceView()->SetProbePosition(p[0], p[1], p[2]);
		}
		else {
			int p[3];
			VolumePick(p); 
			vis->GetVolumeView()->SetProbePosition(p[0], p[1], p[2]);
			vis->GetSliceView()->SetProbePosition(p[0], p[1], p[2]);		
		}

		// Update probes
		vis->GetVolumeView()->SetShowProbe(true);
		vis->GetSliceView()->SetShowProbe(true);
	}
	else {
		vis->GetVolumeView()->SetShowProbe(false);
		vis->GetSliceView()->SetShowProbe(false);
	}

	vis->Render();
}

int InteractionCallbacks::Pick(vtkRenderWindowInteractor* rwi) {
	int x = rwi->GetEventPosition()[0];
	int y = rwi->GetEventPosition()[1];

	return picker->Pick(x, y, 0.0, rwi->FindPokedRenderer(x, y));
}

void InteractionCallbacks::VolumePick(int p[3]) {
	double p0[3];
	picker->GetPickPosition(p0);

	p[0] = round(p0[0]);
	p[1] = round(p0[1]);
	p[2] = round(p0[2]);
}

void InteractionCallbacks::SlicePick(int p[3]) {
	picker->GetPointIJK(p);
}