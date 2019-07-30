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
	this->volumePipeline = nullptr;
	this->slicePipeline = nullptr;
}

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::~vtkInteractorStyleSlice() {
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartPaint()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_PAINTSLICE);

/*
	// Get the last (the topmost) image
	this->SetCurrentImageNumber(this->CurrentImageNumber);

	if (this->HandleObservers &&
		this->HasObserver(vtkCommand::StartWindowLevelEvent))
	{
		this->InvokeEvent(vtkCommand::StartWindowLevelEvent, this);
	}
	else
	{
		if (this->CurrentImageProperty)
		{
			vtkImageProperty *property = this->CurrentImageProperty;
			this->WindowLevelInitial[0] = property->GetColorWindow();
			this->WindowLevelInitial[1] = property->GetColorLevel();
		}
	}
*/
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndPaint()
{
	if (this->State != VTKIS_PAINTSLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		//this->InvokeEvent(vtkCommand::EndWindowLevelEvent, this);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnLeftButtonDown() {
	this->MouseMoved = false;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	// Redefine this button to handle painting
//	this->GrabFocus(this->EventCallbackCommand);

	if (this->Interactor->GetAltKey()) {		
		this->StartPaint();
	}
	else
	{
		this->Superclass::OnLeftButtonDown();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnLeftButtonUp() {
	if (!this->MouseMoved && this->slicePipeline) {
		// Pick at the mouse location provided by the interactor
		int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

		if (pick) {
			// Get the point coordinate for the pick event
			int* p = this->Picker->GetPointIJK();

			if (this->State == VTKIS_PAINTSLICE) {
				this->slicePipeline->Paint(p[0], p[1], p[2]);
			}
			else {
				this->slicePipeline->PickLabel(p[0], p[1], p[2]);
				this->volumePipeline->SetLabel(this->slicePipeline->GetLabel());
			}

			this->slicePipeline->Render();
			this->volumePipeline->Render();
		}
		else {
			this->slicePipeline->SetLabel(0);
			this->volumePipeline->SetLabel(0);

			this->slicePipeline->Render();
			this->volumePipeline->Render();
		}
	}
	
	if (this->State == VTKIS_PAINTSLICE) {
		this->EndPaint();
		if (this->Interactor)
		{
			this->ReleaseFocus();
		}
	}
	
	this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnMouseMove() {
	if (!this->CurrentRenderer) return;

	this->MouseMoved = true;


	// Pick at the mouse location provided by the interactor
	int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

	if (pick && this->slicePipeline && this->volumePipeline) {
		// Get the point coordinate for the pick event
		int* p = this->Picker->GetPointIJK();

		// XXX: Probably cleaner to use observers for events, rather than passing in the pipelines

		// Update probes
		this->volumePipeline->SetProbeVisiblity(true);
		this->slicePipeline->SetProbeVisiblity(true);

		this->slicePipeline->SetProbePosition(p[0], p[1], p[2]);
		this->volumePipeline->SetProbePosition(p[0], p[1], p[2]);
	}
	else {
		this->volumePipeline->SetProbeVisiblity(false);
		this->slicePipeline->SetProbeVisiblity(false);
	}

	if (this->State == VTKIS_PAINTSLICE) {
		this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
		this->Paint();
		//this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
	}

	this->volumePipeline->Render();
	this->slicePipeline->Render();

	this->Superclass::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnEnter() {
	std::cout << "ENTER" << std::endl;
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
void vtkInteractorStyleSlice::Paint()
{
	// Pick at the mouse location provided by the interactor
	int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

	if (pick && this->slicePipeline) {
		// Get the point coordinate for the pick event
		int* p = this->Picker->GetPointIJK();
		this->slicePipeline->Paint(p[0], p[1], p[2]);		

		this->slicePipeline->Render();
		this->volumePipeline->Render();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";
}