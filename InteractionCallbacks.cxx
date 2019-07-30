#include "InteractionCallbacks.h"

#include "vtkCamera.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"

bool firstCallback = true;

void volumeCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	if (!firstCallback) return;

	firstCallback = false;

	vtkCamera* volumeCamera = reinterpret_cast<vtkCamera*>(caller);

	vtkRenderer* sliceRenderer = reinterpret_cast<vtkRenderer*>(clientData);
	vtkCamera* sliceCamera = sliceRenderer->GetActiveCamera();

	sliceCamera->SetFocalPoint(volumeCamera->GetFocalPoint());
	sliceCamera->SetPosition(volumeCamera->GetPosition());
	sliceCamera->SetViewUp(volumeCamera->GetViewUp());

	sliceCamera->SetClippingRange(volumeCamera->GetDistance() - 1, volumeCamera->GetDistance() + 1);

	sliceRenderer->GetRenderWindow()->Render();

	firstCallback = true;
}

void sliceCameraChange(vtkObject* caller, unsigned long eventId, void* clientData, void *callData) {
	if (!firstCallback) return;

	firstCallback = false;

	vtkCamera* sliceCamera = reinterpret_cast<vtkCamera*>(caller);

	vtkRenderer* volumeRenderer = reinterpret_cast<vtkRenderer*>(clientData);
	vtkCamera* volumeCamera = volumeRenderer->GetActiveCamera();

	volumeCamera->SetFocalPoint(sliceCamera->GetFocalPoint());
	volumeCamera->SetPosition(sliceCamera->GetPosition());
	volumeCamera->SetViewUp(sliceCamera->GetViewUp());

	volumeRenderer->ResetCameraClippingRange();
	volumeRenderer->GetRenderWindow()->Render();

	firstCallback = true;
}
