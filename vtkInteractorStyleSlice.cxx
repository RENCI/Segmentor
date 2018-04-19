#include "vtkInteractorStyleSlice.h"

#include "vtkAbstractPicker.h"
#include "vtkObjectFactory.h"
#include "vtkPropPicker.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(vtkInteractorStyleSlice);

//----------------------------------------------------------------------------
vtkInteractorStyleSlice::vtkInteractorStyleSlice() {
	this->MouseMoved = false;

	this->Picker = vtkSmartPointer<vtkPropPicker>::New();
	this->Picker->PickFromListOn();
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
		vtkAbstractPicker* picker = this->Interactor->GetPicker();

		picker->Pick(interactor->GetEventPosition()[0], interactor->GetEventPosition()[1], 0, this->CurrentRenderer);
		double picked[3];
		picker->GetPickPosition(picked);

		cout << interactor->GetEventPosition()[0] << " " << interactor->GetEventPosition()[1] << endl;
		cout << picked[0] << " " << picked[1] << " " << picked[2] << endl;
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