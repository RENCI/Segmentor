#include "vtkInteractorStyleSlice.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkMath.h"
#include "vtkImageProperty.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#include "InteractionEnums.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() 
{
	this->MouseMoved = false;
	this->Mode = NavigationMode;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
}

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::~vtkInteractorStyleSlice() 
{
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::SetMode(enum InteractionMode mode)
{
	this->Mode = mode;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartSelect()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_SELECT_SLICE);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndSelect()
{
	// Enable select in either mode
	if (!(this->State == VTKIS_SELECT_SLICE || this->State == VTKIS_PAN || this->State == VTKIS_SPIN))
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(SelectLabelEvent, nullptr);
	}
	this->StopState();
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
void vtkInteractorStyleSlice::WindowLevel()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;

	this->WindowLevelCurrentPosition[0] = rwi->GetEventPosition()[0];
	this->WindowLevelCurrentPosition[1] = rwi->GetEventPosition()[1];

	if (this->HandleObservers &&
		this->HasObserver(vtkCommand::WindowLevelEvent))
	{
		this->InvokeEvent(vtkCommand::WindowLevelEvent, this);
	}
	
	if (this->CurrentImageProperty)
	{
		int *size = this->CurrentRenderer->GetSize();

		double window = this->WindowLevelInitial[0];
		double level = this->WindowLevelInitial[1];

		// Compute normalized delta

		double dx = (this->WindowLevelCurrentPosition[0] -
			this->WindowLevelStartPosition[0]) * 4.0 / size[0];
		double dy = (this->WindowLevelStartPosition[1] -
			this->WindowLevelCurrentPosition[1]) * 4.0 / size[1];

		// Scale by current values

		if (fabs(window) > 0.01)
		{
			dx = dx * window;
		}
		else
		{
			dx = dx * (window < 0 ? -0.01 : 0.01);
		}
		if (fabs(level) > 0.01)
		{
			dy = dy * level;
		}
		else
		{
			dy = dy * (level < 0 ? -0.01 : 0.01);
		}

		// Abs so that direction does not flip

		if (window < 0.0)
		{
			dx = -1 * dx;
		}
		if (level < 0.0)
		{
			dy = -1 * dy;
		}

		// Compute new window level

		double newWindow = dx + window;
		double newLevel = level - dy;

		if (newWindow < 0.01)
		{
			newWindow = 0.01;
		}

		this->CurrentImageProperty->SetColorWindow(newWindow);
		this->CurrentImageProperty->SetColorLevel(newLevel);

		this->Interactor->Render();
	}
}

//----------------------------------------------------------------------------
double vtkInteractorStyleSlice::GetWindow() {
	return this->CurrentImageProperty ? this->CurrentImageProperty->GetColorWindow() : 0.0;
}

//----------------------------------------------------------------------------
double vtkInteractorStyleSlice::GetLevel() {
	return this->CurrentImageProperty ? this->CurrentImageProperty->GetColorLevel() : 0.0;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnMouseMove()
{
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

	// If shift is held down, do window/level in either mode
	if (this->Interactor->GetShiftKey())
	{
		this->WindowLevelStartPosition[0] = x;
		this->WindowLevelStartPosition[1] = y;
		this->StartWindowLevel();
	}
	else
	{
		if (this->Mode == EditMode)
		{
			// If ctrl is held down, select the region label
			if (this->Interactor->GetControlKey()) {
				this->StartSelect();
			}

			// Otherwise paint
			else
			{
				this->StartPaint();
			}
		}
		else
		{
			// If ctrl is held down, spin
			if (this->Interactor->GetControlKey())
			{
				this->StartSpin();
			}

			// Otherwise rotate around focal point
			else
			{
				this->StartRotate();
			}
		}
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
		if (this->State == VTKIS_PAINT_SLICE) 
		{
			this->InvokeEvent(PaintEvent, nullptr);
		}
		else if (this->State == VTKIS_SPIN)
		{
			this->EndSelect();
		}
	}

	switch (this->State)
	{
	case VTKIS_SELECT_SLICE:
		this->EndSelect();
		break;

	case VTKIS_PAINT_SLICE:
		this->EndPaint();
		break;
	}

	// Call parent to handle all other states and perform additional work
	
	this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnMiddleButtonDown()
{
	this->MouseMoved = false;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	if (this->Mode == EditMode)
	{
		// Select
		this->StartSelect();
	}
	else 
	{
		// Pan
		this->StartPan();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::OnMiddleButtonUp() {
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	// Enable select in either mode
	if (this->State == VTKIS_SELECT_SLICE || (this->State == VTKIS_PAN && !this->MouseMoved))
	{
		this->EndSelect();
	}

	// Call parent to handle all other states and perform additional work

	this->Superclass::OnMiddleButtonUp();
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

	// Slice if ctrl is held down
	if (this->Interactor->GetControlKey())
	{
		this->CurrentRenderer->ResetCameraClippingRange();
		this->StartSlice();
	}
	else
	{
		if (this->Mode == EditMode)
		{
			// Erase
			this->StartErase();		
		}
		else
		{
			// Zoom
			this->StartDolly();
		}
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
		if (this->State == VTKIS_ERASE_SLICE)
		{
			this->InvokeEvent(EraseEvent, nullptr);			
		}
	}

	if (this->State == VTKIS_ERASE_SLICE)
	{
		this->EndErase();
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

	case 'x':
	case 'X':
	{
		const double right[3] = { 0, 1, 0 };
		const double up[3] = { 0, 0, -1 };

		this->SetOrientation(right, up);
		this->CurrentRenderer->ResetCameraClippingRange();
		this->Interactor->Render();
	}
	break;

	case 'y':
	case 'Y':
	{
		const double right[3] = { 1, 0, 0 };
		const double up[3] = { 0, 0, -1 };

		this->SetOrientation(right, up);
		this->CurrentRenderer->ResetCameraClippingRange();
		this->Interactor->Render();
	}
	break;

	case 'z':
	case 'Z':
	{
		const double right[3] = { 1, 0, 0 };
		const double up[3] = { 0, 1, 0 };

		this->SetOrientation(right, up);
		this->CurrentRenderer->ResetCameraClippingRange();
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
void vtkInteractorStyleSlice::SetOrientation(
	const double leftToRight[3], const double viewUp[3])
{
	// Adapted from vtkInteractorStyleImage
	if (this->CurrentRenderer)
	{
		vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();

		// the cross product points out of the screen
		double vector[3];
		vtkMath::Cross(leftToRight, viewUp, vector);

		// Flip if current view matches
		double camProj[3];
		double camUp[3];
		camera->GetDirectionOfProjection(camProj);
		camera->GetViewUp(camUp);
		if (vector[0] == -camProj[0] && vector[1] == -camProj[1] && vector[2] == -camProj[2] &&
			viewUp[0] == camUp[0] && viewUp[1] == camUp[1] && viewUp[2] == camUp[2])
		{
			vector[0] *= -1;
			vector[1] *= -1;
			vector[2] *= -1;
		}

		double focus[3];
		camera->GetFocalPoint(focus);
		double d = camera->GetDistance();
		camera->SetPosition(focus[0] + d * vector[0],
			focus[1] + d * vector[1],
			focus[2] + d * vector[2]);
		camera->SetFocalPoint(focus);
		camera->SetViewUp(viewUp);
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";

	os << indent << "Picker: " << this->Picker << "\n";
}