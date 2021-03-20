#ifndef SegmentVolumeDialog_H
#define SegmentVolumeDialog_H

#include "ui_SegmentVolumeDialog.h"

class VisualizationContainer;

class SegmentVolumeDialog : public QDialog, private Ui::SegmentVolumeDialog {
	Q_OBJECT
public:
	SegmentVolumeDialog(QWidget* parent, VisualizationContainer* visualizationContainer);
	virtual ~SegmentVolumeDialog();
	
public slots:
	// Menu events
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).	
	virtual void on_updateButton_clicked();

protected:
	// Saved values for resetting
	double otsuThreshold;
	
	VisualizationContainer* visualizationContainer;

	void SegmentVolume();
};

#endif
