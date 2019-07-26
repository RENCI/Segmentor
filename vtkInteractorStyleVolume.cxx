#include "vtkInteractorStyleVolume.h"

#include "vtkCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

#include "SlicePipeline.h"
#include "VolumePipeline.h"

vtkStandardNewMacro(vtkInteractorStyleVolume);

//----------------------------------------------------------------------------
vtkInteractorStyleVolume::vtkInteractorStyleVolume() {
	this->MouseMoved = false;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
	this->volumePipeline = nullptr;
	this->slicePipeline = nullptr;
}

//----------------------------------------------------------------------------
vtkInteractorStyleVolume::~vtkInteractorStyleVolume() {
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnLeftButtonDown() {
	this->MouseMoved = false;

	vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnLeftButtonUp() {
	if (!this->MouseMoved) {
/*
		vtkRenderWindowInteractor* interactor = this->Interactor;

		// Pick at the mouse location provided by the interactor
		int pick = this->Picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

		if (pick && this->slicePipeline && this->volumePipeline) {
			// Get the point coordinate for the pick event
			int* p = this->Picker->GetPointIJK();
			this->slicePipeline->PickLabel(p[0], p[1], p[2]);
		}
*/
	}

	vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnMouseMove() {
	if (!this->CurrentRenderer) return;

	this->MouseMoved = false;

	// Pick at the mouse location provided by the interactor
	int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

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