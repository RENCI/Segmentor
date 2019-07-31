#include "vtkInteractorStyleSlice.h"

#include "vtkCallbackCommand.h"
#include "vtkCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

#include "SlicePipeline.h"
#include "VolumePipeline.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() 
{
	this->MouseMoved = false;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
	this->volumePipeline = nullptr;
	this->slicePipeline = nullptr;
}

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::~vtkInteractorStyleSlice() 
{
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartPaint()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_PAINT_SLICE);

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
	if (this->State != VTKIS_PAINT_SLICE)
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
void vtkInteractorStyleSlice::OnMouseMove() {
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

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

	if (this->State == VTKIS_PAINT_SLICE) {
		this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
		this->Paint();
		//this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
	}

	this->volumePipeline->Render();
	this->slicePipeline->Render();

	this->Superclass::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnLeftButtonDown() 
{
	this->MouseMoved = false;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	// Default to rotation
	this->GrabFocus(this->EventCallbackCommand);
	if (!this->Interactor->GetShiftKey() && !this->Interactor->GetControlKey())
	{
		this->StartRotate();
	}

	// If shift is held down, do window/level
	else if (this->InteractionMode == VTKIS_IMAGE3D &&
		this->Interactor->GetShiftKey())
	{
		this->WindowLevelStartPosition[0] = x;
		this->WindowLevelStartPosition[1] = y;
		this->StartWindowLevel();
	}

	// If ctrl is held down in slicing mode, slice the image
	else if (this->InteractionMode == VTKIS_IMAGE_SLICING &&
		this->Interactor->GetControlKey())
	{
		this->StartSlice();
	}

	// If atl is held down, start painting
	else if (this->Interactor->GetAltKey()) {
		this->StartPaint();
	}

	// The rest of the button + key combinations remain the same

	else
	{
		this->Superclass::OnLeftButtonDown();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnLeftButtonUp() {
	if (!this->MouseMoved && this->slicePipeline) 
	{
		// Pick at the mouse location provided by the interactor
		int pick = this->Picker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

		if (pick) 
		{
			// Get the point coordinate for the pick event
			int* p = this->Picker->GetPointIJK();

			if (this->State == VTKIS_PAINT_SLICE) {
				this->slicePipeline->Paint(p[0], p[1], p[2]);
			}
			else {
				this->slicePipeline->PickLabel(p[0], p[1], p[2]);
				this->volumePipeline->SetLabel(this->slicePipeline->GetLabel());
			}

			this->slicePipeline->Render();
			this->volumePipeline->Render();
		}
		else 
		{
			this->slicePipeline->SetLabel(0);
			this->volumePipeline->SetLabel(0);

			this->slicePipeline->Render();
			this->volumePipeline->Render();
		}
	}
	
	if (this->State == VTKIS_PAINT_SLICE) 
	{
		this->EndPaint();
		if (this->Interactor)
		{
			this->ReleaseFocus();
		}
	}

	// Call parent to handle all other states and perform additional work
	
	this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnChar()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;

	switch (rwi->GetKeyCode())
	{
	}

	this->Superclass::OnChar();
}

/* Version below from vtkInteractorStyle, for reference
void vtkInteractorStyle::OnChar()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;

	switch (rwi->GetKeyCode())
	{
	case 'm':
	case 'M':
		if (this->AnimState == VTKIS_ANIM_OFF)
		{
			this->StartAnimate();
		}
		else
		{
			this->StopAnimate();
		}
		break;

	case 'Q':
	case 'q':
	case 'e':
	case 'E':
		rwi->ExitCallback();
		break;

	case 'f':
	case 'F':
	{
		if (this->CurrentRenderer != nullptr)
		{
			this->AnimState = VTKIS_ANIM_ON;
			vtkAssemblyPath *path = nullptr;
			this->FindPokedRenderer(rwi->GetEventPosition()[0],
				rwi->GetEventPosition()[1]);
			rwi->GetPicker()->Pick(rwi->GetEventPosition()[0],
				rwi->GetEventPosition()[1],
				0.0,
				this->CurrentRenderer);
			vtkAbstractPropPicker *picker;
			if ((picker = vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker())))
			{
				path = picker->GetPath();
			}
			if (path != nullptr)
			{
				rwi->FlyTo(this->CurrentRenderer, picker->GetPickPosition());
			}
			this->AnimState = VTKIS_ANIM_OFF;
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
	}
	break;

	case 'u':
	case 'U':
		rwi->UserCallback();
		break;

	case 'r':
	case 'R':
		this->FindPokedRenderer(rwi->GetEventPosition()[0],
			rwi->GetEventPosition()[1]);
		if (this->CurrentRenderer != nullptr)
		{
			this->CurrentRenderer->ResetCamera();
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
		rwi->Render();
		break;

	case 'w':
	case 'W':
	{
		vtkActorCollection *ac;
		vtkActor *anActor, *aPart;
		vtkAssemblyPath *path;
		this->FindPokedRenderer(rwi->GetEventPosition()[0],
			rwi->GetEventPosition()[1]);
		if (this->CurrentRenderer != nullptr)
		{
			ac = this->CurrentRenderer->GetActors();
			vtkCollectionSimpleIterator ait;
			for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait)); )
			{
				for (anActor->InitPathTraversal(); (path = anActor->GetNextPath()); )
				{
					aPart = static_cast<vtkActor *>(path->GetLastNode()->GetViewProp());
					aPart->GetProperty()->SetRepresentationToWireframe();
				}
			}
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
		rwi->Render();
	}
	break;

	case 's':
	case 'S':
	{
		vtkActorCollection *ac;
		vtkActor *anActor, *aPart;
		vtkAssemblyPath *path;
		this->FindPokedRenderer(rwi->GetEventPosition()[0],
			rwi->GetEventPosition()[1]);
		if (this->CurrentRenderer != nullptr)
		{
			ac = this->CurrentRenderer->GetActors();
			vtkCollectionSimpleIterator ait;
			for (ac->InitTraversal(ait); (anActor = ac->GetNextActor(ait)); )
			{
				for (anActor->InitPathTraversal(); (path = anActor->GetNextPath()); )
				{
					aPart = static_cast<vtkActor *>(path->GetLastNode()->GetViewProp());
					aPart->GetProperty()->SetRepresentationToSurface();
				}
			}
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
		rwi->Render();
	}
	break;

	case '3':
		if (rwi->GetRenderWindow()->GetStereoRender())
		{
			rwi->GetRenderWindow()->StereoRenderOff();
		}
		else
		{
			rwi->GetRenderWindow()->StereoRenderOn();
		}
		rwi->Render();
		break;

	case 'p':
	case 'P':
		if (this->CurrentRenderer != nullptr)
		{
			if (this->State == VTKIS_NONE)
			{
				vtkAssemblyPath *path = nullptr;
				int *eventPos = rwi->GetEventPosition();
				this->FindPokedRenderer(eventPos[0], eventPos[1]);
				rwi->StartPickCallback();
				vtkAbstractPropPicker *picker =
					vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker());
				if (picker != nullptr)
				{
					picker->Pick(eventPos[0], eventPos[1],
						0.0, this->CurrentRenderer);
					path = picker->GetPath();
				}
				if (path == nullptr)
				{
					this->HighlightProp(nullptr);
					this->PropPicked = 0;
				}
				else
				{
					this->HighlightProp(path->GetFirstNode()->GetViewProp());
					this->PropPicked = 1;
				}
				rwi->EndPickCallback();
			}
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
		break;
	}
}
*/

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