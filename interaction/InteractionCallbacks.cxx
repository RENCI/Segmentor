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
#include "vtkInteractorStyleSlice.h"

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

	sliceRenderer->ResetCameraClippingRange();
	//sliceCamera->SetClippingRange(volumeCamera->GetDistance() - 0.5, volumeCamera->GetDistance() + 0.5);

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

void InteractionCallbacks::CameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkCamera* camera = static_cast<vtkCamera*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	double fp[3];
	camera->GetFocalPoint(fp);

	vis->SetFocalPoint(fp[0], fp[1], fp[2]);
}

void InteractionCallbacks::OnChar(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkRenderWindowInteractor*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	switch (rwi->GetKeyCode()) {	
	case 'a':
	case 'A': {
		// Pick at the mouse location provided by the interactor	
		int pick = Pick(rwi);

		if (pick) {
			// Get the position for the pick event		
			double p[3];
			PickPosition(p);

			// XXX: HACK TO GUARD AGAINST INVALID VOLUME PICK
			//		NEED TO FIX VOLUME PICKING AND DIFFERENTIATE BETWEEN THEM FOR KEYSTROKE CALLBACK

			if (p[0] == 0 && p[1] == 0 && p[2] == 0) break;

			vis->CreateNewRegion(p);
		}
		break;
	}

	case 'g':
	case 'G': {
		// Pick at the mouse location provided by the interactor	
		int pick = Pick(rwi);

		if (pick) {
			// Get the position for the pick event		
			double p[3];
			PickPosition(p);

			// XXX: HACK TO GUARD AGAINST INVALID VOLUME PICK
			//		NEED TO FIX VOLUME PICKING AND DIFFERENTIATE BETWEEN THEM FOR KEYSTROKE CALLBACK

			if (p[0] == 0 && p[1] == 0 && p[2] == 0) break;
				
			vis->GrowCurrentRegion(p);
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
			// Get the position for the pick event		
			double p[3];
			PickPosition(p);

			// XXX: HACK TO GUARD AGAINST INVALID VOLUME PICK
			//		NEED TO FIX VOLUME PICKING AND DIFFERENTIATE BETWEEN THEM FOR KEYSTROKE CALLBACK

			if (p[0] == 0 && p[1] == 0 && p[2] == 0) break;

			vis->MergeWithCurrentRegion(p);
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

void InteractionCallbacks::SelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double p[3];
		PickPosition(p);
		vis->PickLabel(p);
		vis->Render();
	}
}

void InteractionCallbacks::Paint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double p[3];
		PickPosition(p);
		vis->Paint(p);
		vis->Render();
	}
}

void InteractionCallbacks::Overwrite(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double p[3];
		PickPosition(p);
		vis->Paint(p, true);
		vis->Render();
	}
}

void InteractionCallbacks::Erase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double p[3];
		PickPosition(p);
		vis->Erase(p);
		vis->Render();
	}
}

void InteractionCallbacks::MouseMove(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkRenderWindowInteractor*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event
		double p[3];
		PickPosition(p);

		vis->MouseMove(p);
	}
	else {
		vis->MouseMove();
	}

	vis->Render();
}

int InteractionCallbacks::Pick(vtkRenderWindowInteractor* rwi) {
	int x = rwi->GetEventPosition()[0];
	int y = rwi->GetEventPosition()[1];

	return picker->Pick(x, y, 0.0, rwi->FindPokedRenderer(x, y));
}

void InteractionCallbacks::PickPosition(double p[3]) {
	picker->GetPickPosition(p);
}

void InteractionCallbacks::WindowLevel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkInteractorStyleSlice* style = static_cast<vtkInteractorStyleSlice*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	vis->SetWindowLevel(style->GetWindow(), style->GetLevel());
}