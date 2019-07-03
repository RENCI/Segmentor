#include "vtkInteractorStyleSlice.h"

#include "vtkCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() {
	this->MouseMoved = false;
	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
	this->Labels = nullptr;
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
	this->MouseMoved = true;

	vtkInteractorStyleImage::OnMouseMove();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleSlice::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);

	os << indent << "Mouse Moved: " << this->MouseMoved << "\n";
}