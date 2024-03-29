#include "InteractionCallbacks.h"

#include "vtkCamera.h"
#include "vtkCell.h"
#include "vtkCellPicker.h"
#include "vtkInteractorStyle.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#include "VisualizationContainer.h"
#include "VolumeView.h"
#include "SliceView.h"
#include "vtkInteractorStyleSlice.h"
#include "vtkInteractorStyleVolume.h"

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

void InteractionCallbacks::SliceSelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
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

void InteractionCallbacks::VolumeSelectLabel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the label for the pick event
		unsigned short label = PickLabel();
		vis->SelectRegion(label);
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

void InteractionCallbacks::EndPaint(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);
	vis->EndPaint();
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

void InteractionCallbacks::EndOverwrite(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);
	vis->EndOverwrite();
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

void InteractionCallbacks::EndErase(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);
	vis->EndErase();
}

void InteractionCallbacks::Add(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event		
		double p[3];
		PickPosition(p);
		vis->CreateNewRegion(p);
	}
}

void InteractionCallbacks::Merge(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event		
		double p[3];
		PickPosition(p);
		vis->MergeWithCurrentRegion(p);
	}
}

void InteractionCallbacks::Grow(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event		
		double p[3];
		PickPosition(p);
		vis->GrowCurrentRegion(p);
	}
}

void InteractionCallbacks::Done(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event		
		double p[3];
		PickPosition(p);
		vis->ToggleRegionDone(p);
	}
}

void InteractionCallbacks::Visible(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event		
		double p[3];
		PickPosition(p);
		vis->ToggleRegionVisibility(p);
	}
}

void InteractionCallbacks::Dot(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkRenderWindowInteractor* rwi = static_cast<vtkInteractorStyle*>(caller)->GetInteractor();
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	// Pick at the mouse location provided by the interactor	
	int pick = Pick(rwi);

	if (pick) {
		// Get the position for the pick event		
		double p[3];
		PickPosition(p);
		vis->SetDotAnnotation(p);
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

unsigned short InteractionCallbacks::PickLabel() {
	vtkDataSet* data = picker->GetDataSet();

	if (!data) return 0;

	vtkCell* cell = data->GetCell(picker->GetCellId());
	vtkVariant value = data->GetPointData()->GetAbstractArray(0)->GetVariantValue(cell->GetPointId(0));

	return value.ToUnsignedShort();
}

void InteractionCallbacks::WindowLevel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkInteractorStyleSlice* style = static_cast<vtkInteractorStyleSlice*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	vis->SetWindowLevel(style->GetWindow(), style->GetLevel());
}

void InteractionCallbacks::VolumeWindowLevel(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	vtkInteractorStyleVolume* style = static_cast<vtkInteractorStyleVolume*>(caller);
	VisualizationContainer* vis = static_cast<VisualizationContainer*>(clientData);

	vis->SetVolumeWindowLevel(style->GetWindow(), style->GetLevel());
}