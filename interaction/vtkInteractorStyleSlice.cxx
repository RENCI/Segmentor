#include "vtkInteractorStyleSlice.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkMath.h"
#include "vtkImageProperty.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
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
	if (!(this->State == VTKIS_SELECT_SLICE || this->State == VTKIS_PAN))
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
void vtkInteractorStyleSlice::StartOverwrite()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_OVERWRITE_SLICE);

	if (this->HandleObservers)
	{
		this->InvokeEvent(StartOverwriteEvent, nullptr);
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndOverwrite()
{
	if (this->State != VTKIS_OVERWRITE_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(EndOverwriteEvent, nullptr);
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
void vtkInteractorStyleSlice::StartAdd()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_ADD_SLICE);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndAdd()
{
	if (this->State != VTKIS_ADD_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(AddEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartMerge()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_MERGE_SLICE);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndMerge()
{
	if (this->State != VTKIS_MERGE_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(MergeEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartGrow()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_GROW_SLICE);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndGrow()
{
	if (this->State != VTKIS_GROW_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(GrowEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartVisible()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_VISIBLE_SLICE);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndVisible()
{
	if (this->State != VTKIS_VISIBLE_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(VisibleEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::StartDot()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_DOT_SLICE);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::EndDot()
{
	if (this->State != VTKIS_DOT_SLICE)
	{
		return;
	}
	if (this->HandleObservers)
	{
		this->InvokeEvent(DotEvent, nullptr);
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

	case VTKIS_OVERWRITE_SLICE:
		this->InvokeEvent(OverwriteEvent, nullptr);
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
			if (this->Interactor->GetControlKey()) 
			{
				this->StartSelect();
			}

			// If alt is held down, overwrite
			else if (this->Interactor->GetAltKey()) 
			{
				this->StartOverwrite();
			}

			// Otherwise paint
			else
			{
				this->StartPaint();
			}
		}
		else if (this->Mode == AddMode)
		{
			this->StartAdd();
		}
		else if (this->Mode == MergeMode)
		{
			this->StartMerge();
		}
		else if (this->Mode == GrowMode)
		{
			this->StartGrow();
		}
		else if (this->Mode == VisibleMode)
		{
			this->StartVisible();
		}
		else if (this->Mode == DotMode)
		{
			this->StartDot();
		}
		else
		{
			// Rotate
			this->StartRotate();
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
		else if (this->State == VTKIS_OVERWRITE_SLICE)
		{
			this->InvokeEvent(OverwriteEvent, nullptr);
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

	case VTKIS_OVERWRITE_SLICE:
		this->EndOverwrite();
		break;

	case VTKIS_ADD_SLICE:
		this->EndAdd();
		break;

	case VTKIS_MERGE_SLICE:
		this->EndMerge();
		break;

	case VTKIS_GROW_SLICE:
		this->EndGrow();
		break;

	case VTKIS_VISIBLE_SLICE:
		this->EndVisible();
		break;

	case VTKIS_DOT_SLICE:
		this->EndDot();
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

	// Ignore defaults
	case 'e':
	case 'E':
	case 'i':
	case 'I':
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
	case 'x':
	case 'X':
	case 'y':
	case 'Y':
	case 'z':
	case 'Z':
	case '3':
		break;

	default:
		this->Superclass::OnChar();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::Rotate()
{
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor *rwi = this->Interactor;

	int dx = this->Interactor->GetControlKey() ? 0 :
		rwi->GetEventPosition()[0] - rwi->GetLastEventPosition()[0];
	int dy = this->Interactor->GetAltKey() ? 0 :
		rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

	int *size = this->CurrentRenderer->GetRenderWindow()->GetSize();

	double delta_elevation = -20.0 / size[1];
	double delta_azimuth = -20.0 / size[0];

	double rxf = dx * delta_azimuth * this->MotionFactor;
	double ryf = dy * delta_elevation * this->MotionFactor;

	vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
	camera->Azimuth(rxf);
	camera->Elevation(ryf);
	camera->OrthogonalizeViewUp();

	if (this->AutoAdjustCameraClippingRange)
	{
		this->CurrentRenderer->ResetCameraClippingRange();
	}

	if (rwi->GetLightFollowCamera())
	{
		this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
	}

	rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";

	os << indent << "Picker: " << this->Picker << "\n";
}