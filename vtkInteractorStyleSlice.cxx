#include "vtkInteractorStyleSlice.h"

#include "vtkCallbackCommand.h"
#include "vtkCellPicker.h"
#include "vtkImageProperty.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() 
{
	this->MouseMoved = false;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
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

	if (this->HandleObservers)
	{
		this->InvokeEvent(StartPaintEvent, nullptr);
	}
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
		this->InvokeEvent(EndPaintEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartErase()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_ERASE_SLICE);

	if (this->HandleObservers)
	{
		this->InvokeEvent(StartEraseEvent, nullptr);
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndErase()
{
	if (this->State != VTKIS_ERASE_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(EndEraseEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnMouseMove() {
	this->MouseMoved = true;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1]; 

	switch (this->State)
	{
	case VTKIS_PAINT_SLICE:		
		this->InvokeEvent(PaintEvent, nullptr);
		break;

	case VTKIS_ERASE_SLICE:
		this->InvokeEvent(EraseEvent, nullptr);
		break;
	}

	// Call parent to handle all other states and perform additional work

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
	if (!this->Interactor->GetShiftKey() && 
		!this->Interactor->GetControlKey() && 
		!this->Interactor->GetAltKey())
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

	// If alt is held down, start painting
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
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}
	
	if (!this->MouseMoved)
	{
		switch (this->State) 
		{
		case VTKIS_PAINT_SLICE:
			this->InvokeEvent(PaintEvent, nullptr);
			break;

		default:
			this->InvokeEvent(SelectLabelEvent, nullptr);
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
void vtkInteractorStyleSlice::OnRightButtonDown()
{
	this->MouseMoved = false;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	// If alt is held down, start erasing
	if (this->Interactor->GetAltKey()) {
		this->StartErase();
	}

	// The rest of the button + key combinations remain the same

	else
	{
		this->Superclass::OnRightButtonDown();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnRightButtonUp() {
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	if (!this->MouseMoved)
	{
		switch (this->State)
		{
		case VTKIS_ERASE_SLICE:
			this->InvokeEvent(EraseEvent, nullptr);
			break;

		default:
			this->InvokeEvent(SelectLabelEvent, nullptr);
		}
	}

	if (this->State == VTKIS_ERASE_SLICE)
	{
		this->EndErase();
		if (this->Interactor)
		{
			this->ReleaseFocus();
		}
	}


	// Call parent to handle all other states and perform additional work

	this->Superclass::OnRightButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnChar()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;

	switch (rwi->GetKeyCode())
	{
	case 'r':
	case 'R':
		// Shift triggers reset window level event
		if (!rwi->GetShiftKey())
		{
			// Use standard interactor style view reset
			vtkInteractorStyle::OnChar();
		}
		else if (this->HandleObservers &&
			this->HasObserver(vtkCommand::ResetWindowLevelEvent))
		{
			this->InvokeEvent(vtkCommand::ResetWindowLevelEvent, this);
		}
		else if (this->CurrentImageProperty)
		{
			vtkImageProperty *property = this->CurrentImageProperty;
			property->SetColorWindow(this->WindowLevelInitial[0]);
			property->SetColorLevel(this->WindowLevelInitial[1]);
			this->Interactor->Render();
		}
		break;

	// Ignore defaults
	case 'e':
	case 'E':
	case 'm':
	case 'M':
	case 'p':
	case 'P':
	case 's':
	case 'S':
	case 'u':
	case 'U':
	case 'w':
	case 'W':
	case '3':
		break;

	default:
		this->Superclass::OnChar();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";

	os << indent << "Picker: " << this->Picker << "\n";
}