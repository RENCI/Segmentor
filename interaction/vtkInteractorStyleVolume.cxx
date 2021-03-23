#include "vtkInteractorStyleVolume.h"

#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellPicker.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
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
void vtkInteractorStyleVolume::StartSelect()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_SELECT_VOLUME);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::EndSelect()
{
	// Enable select in either mode
	if (!(this->State == VTKIS_SELECT_VOLUME || this->State == VTKIS_PAN))
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
void vtkInteractorStyleVolume::StartMerge()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_MERGE_VOLUME);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::EndMerge()
{
	if (this->State != VTKIS_MERGE_VOLUME)
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
void vtkInteractorStyleVolume::StartVisible()
{
	if (this->State != VTKIS_NONE)
	{
		return;
	}
	this->StartState(VTKIS_VISIBLE_VOLUME);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::EndVisible()
{
	if (this->State != VTKIS_VISIBLE_VOLUME)
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
	else if (this->Mode == MergeMode)
	{
		this->StartMerge();
	}
	else if (this->Mode == VisibleMode)
	{
		this->StartVisible();
	}
	else
	{
		// Rotate
		this->StartRotate();
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
		if (this->State == VTKIS_PAINT_VOLUME)
		{
			this->InvokeEvent(PaintEvent, nullptr);
		}
	}

	switch (this->State)
	{
	case VTKIS_SELECT_VOLUME:
		this->EndSelect();
		break;

	case VTKIS_PAINT_VOLUME:
		this->EndPaint();
		break;

	case VTKIS_MERGE_VOLUME:
		this->EndMerge();
		break;
		
	case VTKIS_VISIBLE_VOLUME:
		this->EndVisible();
		break;
	}

	// Call parent to handle all other states and perform additional work

	this->Superclass::OnLeftButtonUp();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::OnMiddleButtonDown()
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
void vtkInteractorStyleVolume::OnMiddleButtonUp() {
	int x = this->Interactor->GetEventPosition()[0];
	int y = this->Interactor->GetEventPosition()[1];

	this->FindPokedRenderer(x, y);
	if (this->CurrentRenderer == nullptr)
	{
		return;
	}

	// Enable select in either mode
	if (this->State == VTKIS_SELECT_VOLUME || (this->State == VTKIS_PAN && !this->MouseMoved))
	{
		this->EndSelect();
	}

	// Call parent to handle all other states and perform additional work

	this->Superclass::OnMiddleButtonUp();
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
				this->FlyTo(picker->GetPickPosition());
			}
			this->AnimState = VTKIS_ANIM_OFF;
		}
		else
		{
			vtkWarningMacro(<< "no current renderer on the interactor style.");
		}
	}
	break;

	case 'i':
	case 'I':
	{
		if (this->CurrentRenderer != nullptr)
		{
			this->CurrentRenderer->ResetCamera();
			this->Interactor->Render();
		}
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
		break;
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::Rotate()
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

//----------------------------------------------------------------------
void vtkInteractorStyleVolume::FlyTo(double flyTo[3])
{
	if (this->CurrentRenderer == nullptr) {
		this->FindPokedRenderer(0, 0);

		if (this->CurrentRenderer == nullptr)
		{
			return;
		}
	}

	vtkRenderer* ren = this->CurrentRenderer;
	int frames = this->Interactor->GetNumberOfFlyFrames();

	double flyFrom[3], direction[3], d[3], focalPt[3], position[3];
	int i, j;

	ren->GetActiveCamera()->GetFocalPoint(flyFrom);
	ren->GetActiveCamera()->GetDirectionOfProjection(direction);
	double focalDistance = ren->GetActiveCamera()->GetDistance();

	for (i = 0; i < 3; i++)
	{
		d[i] = flyTo[i] - flyFrom[i];
	}

	double distance = vtkMath::Normalize(d);
	double delta = distance / frames;

	for (i = 1; i <= frames; i++)
	{
		for (j = 0; j < 3; j++)
		{
			focalPt[j] = (int)(flyFrom[j] + d[j] * i * delta);
			position[j] = focalPt[j] - direction[j] * focalDistance;
		}

		ren->GetActiveCamera()->SetFocalPoint(focalPt);
		ren->GetActiveCamera()->SetPosition(position);
		ren->GetActiveCamera()->OrthogonalizeViewUp();
		ren->ResetCameraClippingRange();
		this->Interactor->Render();
	}
}

//----------------------------------------------------------------------------
void vtkInteractorStyleVolume::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";

	os << indent << "Picker: " << this->Picker << "\n";
}