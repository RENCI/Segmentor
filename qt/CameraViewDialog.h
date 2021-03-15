#ifndef CameraViewDialog_H
#define CameraViewDialog_H

#include "ui_CameraViewDialog.h"

class vtkRenderer;

class CameraViewDialog : public QDialog, private Ui::CameraViewDialog {
	Q_OBJECT
public:
	CameraViewDialog(QWidget* parent, vtkRenderer* renderer);
	virtual ~CameraViewDialog();
	
public slots:
	// Menu events
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).

	virtual void on_cameraXSpinBox_valueChanged(double);
	virtual void on_cameraYSpinBox_valueChanged(double);
	virtual void on_cameraZSpinBox_valueChanged(double);

	virtual void on_sliceXSpinBox_valueChanged(double);
	virtual void on_sliceYSpinBox_valueChanged(double);
	virtual void on_sliceZSpinBox_valueChanged(double);

protected:
	vtkRenderer * renderer;

	void updateCameraPosition();
	void updateSlicePosition();

	void initializeCameraValues(double values[3]);
	void initializeSliceValues(double values[3]);
};

#endif
