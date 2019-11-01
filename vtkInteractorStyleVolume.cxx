#include "vtkInteractorStyleVolume.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

#include "InteractionEnums.h"

vtkStandardNewMacro(vtkInteractorStyleVolume);

//----------------------------------------------------------------------------
vtkInteractorStyleVolume::vtkInteractorStyleVolume() 
{
	this->MouseMoved = false;
	this->Mode = NavigationMode;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
}

//----------------------------------------------------------------------------
vtkInteractorStyleVolume::~vtkInteractorStyleVolume() 
{
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::SetMode(enum InteractionMode mode)
{
	this->Mode = mode;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::StartPaint()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_PAINT_VOLUME);

	if (this->HandleObservers)
	{
		this->InvokeEvent(StartPaintEvent, nullptr);
	}
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
		this->InvokeEvent(EndPaintEvent, nullptr);
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::StartErase()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_ERASE_VOLUME);

	if (this->HandleObservers)
	{
		this->InvokeEvent(StartEraseEvent, nullptr);
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::EndErase()
{
	if (this->State != VTKIS_ERASE_VOLUME)
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
void vtkInteractorStyleVolume::StartSlice()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_SLICE_VOLUME);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::EndSlice()
{
	if (this->State != VTKIS_SLICE_VOLUME)
	{
		return;
	}
	this->StopState();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnMouseMove() 
{
	this->MouseMoved = true;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	switch (this->State)
	{
	case VTKIS_PAINT_VOLUME:
		this->InvokeEvent(PaintEvent, nullptr);
		break;

	case VTKIS_ERASE_VOLUME:
		this->InvokeEvent(EraseEvent, nullptr);
		break;

	case VTKIS_SLICE_VOLUME:
		this->FindPokedRenderer(x, y);
		this->Slice();
		break;
	}

	// Call parent to handle all other states and perform additional work

	this->Superclass::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnLeftButtonDown() 
{
	this->MouseMoved = false;

	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}
		
	// If atl is held down, start painting
	if (this->Interactor->GetAltKey()) {
		this->StartPaint();
	}

	// The rest of the button + key combinations remain the same

	else
	{
		this->Superclass::OnLeftButtonDown();
		this->ReleaseFocus();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnLeftButtonUp() 
{
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
		case VTKIS_PAINT_VOLUME:
			this->InvokeEvent(PaintEvent, nullptr);
			break;

		case VTKIS_ROTATE:
			this->InvokeEvent(SelectLabelEvent, nullptr);
		}
	}

	if (this->State == VTKIS_PAINT_VOLUME)
	{
		this->EndPaint();
	}

	// Call parent to handle all other states and perform additional work

	this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnRightButtonDown()
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
	if (this->Interactor->GetAltKey()) 
	{
		this->StartErase();
	}

	// If ctl is held down, move the slice plane via the camera focal point
	else if (this->Interactor->GetControlKey())
	{
		this->StartSlice();
	}

	// The rest of the button + key combinations remain the same

	else
	{
		this->Superclass::OnRightButtonDown();
		this->ReleaseFocus();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnRightButtonUp() {
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	if (!this->MouseMoved)
	{
		if (this->State == VTKIS_ERASE_VOLUME)
		{
			this->InvokeEvent(EraseEvent, nullptr);
		}
	}

	switch (this->State)
	{ 
	case VTKIS_ERASE_VOLUME:	
		this->EndErase();
		break;

	case VTKIS_SLICE_VOLUME:
		this->EndSlice();
		break;
	}

	// Call parent to handle all other states and perform additional work

	this->Superclass::OnRightButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnChar()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;

	switch (rwi->GetKeyCode())	
	{
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
		break;
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::Slice()
{
	// Borrowed from vtkInteractorStyleImage
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	vtkRenderWindowInteractor *rwi = this->Interactor;
	int dy = rwi->GetEventPosition()[1] - rwi->GetLastEventPosition()[1];

	vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
	double *range = camera->GetClippingRange();
	double distance = camera->GetDistance();

	// scale the interaction by the height of the viewport
	double viewportHeight = 0.0;
	if (camera->GetParallelProjection())
	{
		viewportHeight = camera->GetParallelScale();
	}
	else
	{
		double angle = vtkMath::RadiansFromDegrees(camera->GetViewAngle());
		viewportHeight = 2.0*distance*tan(0.5*angle);
	}

	int *size = this->CurrentRenderer->GetSize();
	double delta = dy * viewportHeight / size[1];
	distance += delta;

	// clamp the distance to the clipping range
	if (distance < range[0])
	{
		distance = range[0] + viewportHeight * 1e-3;
	}
	if (distance > range[1])
	{
		distance = range[1] - viewportHeight * 1e-3;
	}
	camera->SetDistance(distance);

	rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::SetOrientation(
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
void vtkInteractorStyleVolume::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";

	os << indent << "Picker: " << this->Picker << "\n";
}