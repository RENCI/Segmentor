#include "vtkInteractorStyleVolume.h"

#include "vtkCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

#include "SlicePipeline.h"
#include "VolumePipeline.h"

vtkStandardNewMacro(vtkInteractorStyleVolume);

//----------------------------------------------------------------------------
vtkInteractorStyleVolume::vtkInteractorStyleVolume() {
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
	this->volumePipeline = nullptr;
	this->slicePipeline = nullptr;
}

//----------------------------------------------------------------------------
vtkInteractorStyleVolume::~vtkInteractorStyleVolume() {
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnLeftButtonDown() {
	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnLeftButtonUp() {
	vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnMouseMove() {
	if (!this->CurrentRenderer) return;

	vtkRenderWindowInteractor* interactor = this->GetInteractor();

	// Pick at the mouse location provided by the interactor
	int pick = this->Picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

	if (pick && this->volumePipeline && this->slicePipeline) {
		// Get the point coordinate for the pick event
		double* p = this->Picker->GetPickPosition();

		// Update probes
		this->volumePipeline->ShowProbe();
		this->slicePipeline->ShowProbe();

		this->volumePipeline->SetProbePosition(p[0], p[1], p[2]);
		this->slicePipeline->SetProbePosition(p[0], p[1], p[2]);

		this->volumePipeline->Render();
		this->slicePipeline->Render();
	}
	else {
		this->volumePipeline->ShowProbe(false);
		this->slicePipeline->ShowProbe(false);

		this->volumePipeline->Render();
		this->slicePipeline->Render();
	}

	vtkInteractorStyleTrackballCamera::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::SetVolumePipeline(VolumePipeline* pipeline) {
	this->volumePipeline = pipeline;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::SetSlicePipeline(SlicePipeline* pipeline) {
	this->slicePipeline = pipeline;
}


//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);
}