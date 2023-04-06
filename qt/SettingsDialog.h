#ifndef SettingsDialog_H
#define SettingsDialog_H

#include "ui_SettingsDialog.h"

class VisualizationContainer;

class SettingsDialog : public QDialog, private Ui::SettingsDialog {
	Q_OBJECT
public:
	SettingsDialog(QWidget* parent, VisualizationContainer* visualizationContainer);
	virtual ~SettingsDialog();

	void initializeSettings();
	
public slots:
	// Menu events
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).
	
	virtual void on_windowSpinBox_valueChanged(double value);
	virtual void on_levelSpinBox_valueChanged(double value);

	virtual void on_overlayOpacitySpinBox_valueChanged(double value);
	virtual void on_overlayOpacityUp();
	virtual void on_overlayOpacityDown();

	virtual void on_surfaceOpacitySpinBox_valueChanged(double value);
	virtual void on_surfaceOpacityUp();
	virtual void on_surfaceOpacityDown();

	virtual void on_neighborRadiusSpinBox_valueChanged(double value);

	virtual void on_voxelSizeSpinBox();

	virtual void on_windowLevelChanged(double window, double level);
	virtual void on_overlayOpacityChanged(double value);
	virtual void on_surfaceOpacityChanged(double value);

	virtual void on_gradientOpacityCheckBox_stateChanged(int state);
	virtual void on_autoAdjustSamplingCheckBox_stateChanged(int state);

	virtual void on_enableDotAnnotationCheckBox_stateChanged(int state);
	virtual void on_dotSizeSpinBox_valueChanged(double value);

	virtual void on_flipAxisButton(int axis);

signals:
	void enableDotAnnotationChanged(bool enable);

protected:
	VisualizationContainer* visualizationContainer;
};

#endif
