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
void vtkInteractorStyleVolume::StartPaint()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_PAINT_VOLUME);

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
void vtkInteractorStyleVolume::EndPaint()
{
	if (this->State != VTKIS_PAINT_VOLUME)
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
void vtkInteractorStyleVolume::OnLeftButtonDown() {
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
void vtkInteractorStyleVolume::OnLeftButtonUp() {
	if (!this->MouseMoved && this->slicePipeline) {
		// Pick at the mouse location provided by the interactor
		int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

		if (pick) {
			// Get the point coordinate for the pick event
			double* p = this->Picker->GetPickPosition();

			if (this->State == VTKIS_PAINT_VOLUME) {
				this->slicePipeline->PaintPoint(p[0], p[1], p[2]);
			}
			else {
				this->slicePipeline->PickPointLabel(p[0], p[1], p[2]);
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

	if (this->State == VTKIS_PAINT_VOLUME) {
		this->EndPaint();
		if (this->Interactor)
		{
			this->ReleaseFocus();
		}
	}

	this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnChar()
{
	switch (this->Interactor->GetKeyCode())	{
	case 's':
	case 'S':
		this->volumePipeline->ToggleSmoothSurfaces();
		break;

	default:
		this->Superclass::OnChar();
		break;
	}
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
		this->volumePipeline->SetProbeVisiblity(true);
		this->slicePipeline->SetProbeVisiblity(true);

		this->volumePipeline->SetProbePosition(p[0], p[1], p[2]);
		this->slicePipeline->SetProbePosition(p[0], p[1], p[2]);
	}
	else {
		this->volumePipeline->SetProbeVisiblity(false);
		this->slicePipeline->SetProbeVisiblity(false);
	}

	if (this->State == VTKIS_PAINT_VOLUME) {
		this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
		this->Paint();
		//this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
	}

	this->volumePipeline->Render();
	this->slicePipeline->Render();

	this->Superclass::OnMouseMove();
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
void vtkInteractorStyleVolume::Paint()
{
	// Pick at the mouse location provided by the interactor
	int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

	if (pick && this->slicePipeline) {
		// Get the point coordinate for the pick event
		double* p = this->Picker->GetPickPosition();
		this->slicePipeline->PaintPoint(p[0], p[1], p[2]);

		this->slicePipeline->Render();
		this->volumePipeline->Render();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";
}