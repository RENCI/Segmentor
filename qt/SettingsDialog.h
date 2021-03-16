#ifndef SettingsDialog_H
#define SettingsDialog_H

#include "ui_SettingsDialog.h"

class VisualizationContainer;

class SettingsDialog : public QDialog, private Ui::SettingsDialog {
	Q_OBJECT
public:
	SettingsDialog(QWidget* parent, VisualizationContainer* visualizationContainer);
	virtual ~SettingsDialog();
	
public slots:
	// Menu events
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).
	
	virtual void on_windowSpinBox_valueChanged(double value);
	virtual void on_levelSpinBox_valueChanged(double value);

	virtual void on_overlaySpinBox_valueChanged(double value);
	virtual void on_overlayUp();
	virtual void on_overlayDown();

	virtual void on_opacitySpinBox_valueChanged(double value);
	virtual void on_opacityUp();
	virtual void on_opacityDown();

	virtual void on_voxelSizeSpinBox();

	virtual void on_brushRadiusSpinBox_valueChanged(int value);
	virtual void on_brushRadiusUp();
	virtual void on_brushRadiusDown();	

	virtual void on_reject();

protected:
	// Saved values for cancelling
	double xSize;
	double ySize;
	double zSize;

	double window;
	double level;

	double overlayOpacity;
	double regionOpacity;
	double brushRadius;

	VisualizationContainer* visualizationContainer;
};

#endif
