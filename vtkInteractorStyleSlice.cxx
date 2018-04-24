#include "vtkInteractorStyleSlice.h"

#include "vtkCellPicker.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() {
	this->MouseMoved = false;

	this->Picker = vtkSmartPointer<vtkCellPicker>::New();
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
		vtkRenderWindowInteractor* interactor = this->GetInteractor();

		// Pick at the mouse location provided by the interactor
		int pick = this->Picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

		if (pick) {
			int* p = this->Picker->GetPointIJK();

			cout << p[0] << " " << p[1] << " " << p[2] << endl;
		}
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