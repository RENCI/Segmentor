#include "CameraViewDialog.h"

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <iostream>

// Constructor
CameraViewDialog::CameraViewDialog(QWidget* parent, vtkRenderer* renderer)
: QDialog(parent), renderer(renderer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	vtkCamera* camera = renderer->GetActiveCamera();
	initializeCameraValues(camera->GetPosition());
	initializeSliceValues(camera->GetFocalPoint());
}

CameraViewDialog::~CameraViewDialog() {
}

void CameraViewDialog::on_cameraXSpinBox_valueChanged(double) {
	updateCameraPosition();
}

void CameraViewDialog::on_cameraYSpinBox_valueChanged(double) {
	updateCameraPosition();
}

void CameraViewDialog::on_cameraZSpinBox_valueChanged(double) {
	updateCameraPosition();
}

void CameraViewDialog::on_sliceXSpinBox_valueChanged(double) {
	updateSlicePosition();
}

void CameraViewDialog::on_sliceYSpinBox_valueChanged(double) {
	updateSlicePosition();
}

void CameraViewDialog::on_sliceZSpinBox_valueChanged(double) {
	updateSlicePosition();
}

void CameraViewDialog::updateCameraPosition() {
	double pos[3];
	pos[0] = cameraXSpinBox->value();
	pos[1] = cameraYSpinBox->value();
	pos[2] = cameraZSpinBox->value();

	renderer->GetActiveCamera()->SetPosition(pos);
	renderer->GetActiveCamera()->OrthogonalizeViewUp(); 
	renderer->ResetCameraClippingRange();

	renderer->GetRenderWindow()->Render();
}

void CameraViewDialog::updateSlicePosition() {
	double pos[3];
	pos[0] = sliceXSpinBox->value();
	pos[1] = sliceYSpinBox->value();
	pos[2] = sliceZSpinBox->value();

	renderer->GetActiveCamera()->SetFocalPoint(pos);
	renderer->GetActiveCamera()->OrthogonalizeViewUp();
	renderer->ResetCameraClippingRange();

	renderer->GetRenderWindow()->Render();
}

void CameraViewDialog::initializeCameraValues(double values[3]) {
	cameraXSpinBox->blockSignals(true);
	cameraYSpinBox->blockSignals(true);
	cameraZSpinBox->blockSignals(true);

	cameraXSpinBox->setValue(values[0]);
	cameraYSpinBox->setValue(values[1]);
	cameraZSpinBox->setValue(values[2]);

	cameraXSpinBox->blockSignals(false);
	cameraYSpinBox->blockSignals(false);
	cameraZSpinBox->blockSignals(false);
}

void CameraViewDialog::initializeSliceValues(double values[3]) {
	sliceXSpinBox->blockSignals(true);
	sliceYSpinBox->blockSignals(true);
	sliceZSpinBox->blockSignals(true);

	sliceXSpinBox->setValue(values[0]);
	sliceYSpinBox->setValue(values[1]);
	sliceZSpinBox->setValue(values[2]);

	sliceXSpinBox->blockSignals(false);
	sliceYSpinBox->blockSignals(false);
	sliceZSpinBox->blockSignals(false);
}