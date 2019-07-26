#include "vtkInteractorStyleSlice.h"

#include "vtkCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

#include "SlicePipeline.h"
#include "VolumePipeline.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() {
	this->MouseMoved = false;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
	this->Labels = nullptr;
	this->volumePipeline = nullptr;
	this->slicePipeline = nullptr;
}

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::~vtkInteractorStyleSlice() {
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnLeftButtonDown() {
	this->MouseMoved = false;

	vtkInteractorStyleImage::OnLeftButtonDown();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnLeftButtonUp() {
	if (!this->MouseMoved) {
/*
		vtkRenderWindowInteractor* interactor = this->GetInteractor();

		// Pick at the mouse location provided by the interactor
		int pick = this->Picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

		if (pick) {
			// Get the point coordinate for the pick event
			int* p = this->Picker->GetPointIJK();

			// Toggle the label
			unsigned int* label = static_cast<unsigned int*>(this->Labels->GetScalarPointer(p));
			label[0] = label[0] > 0 ? 0 : 1;
			this->Labels->Modified();

			// XXX: May need to create a vtk filter wrapper around the image data to make sure it is updated correctly

			// Render
			interactor->Render();
		}
*/
	}

	vtkInteractorStyleImage::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnMouseMove() {
	if (!this->CurrentRenderer) return;

	this->MouseMoved = true;

	vtkRenderWindowInteractor* interactor = this->GetInteractor();

	// Pick at the mouse location provided by the interactor
	int pick = this->Picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

	if (pick && this->slicePipeline && this->volumePipeline) {
		// Get the point coordinate for the pick event
		int* p = this->Picker->GetPointIJK();

		// Update probes
		this->volumePipeline->ShowProbe();
		this->slicePipeline->ShowProbe();

		this->slicePipeline->SetProbePosition(p[0], p[1], p[2]);
		this->volumePipeline->SetProbePosition(p[0], p[1], p[2]);

		this->slicePipeline->Render();
		this->volumePipeline->Render();
	}
	else {
		this->volumePipeline->ShowProbe(false);
		this->slicePipeline->ShowProbe(false);

		this->slicePipeline->Render();
		this->volumePipeline->Render();
	}

	vtkInteractorStyleImage::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::SetVolumePipeline(VolumePipeline* pipeline) {
	this->volumePipeline = pipeline;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::SetSlicePipeline(SlicePipeline* pipeline) {
	this->slicePipeline = pipeline;
}


//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";
}