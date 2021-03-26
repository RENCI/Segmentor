#ifndef SplitRegionDialog_H
#define SplitRegionDialog_H

#include "ui_SplitRegionDialog.h"

class VisualizationContainer;

class SplitRegionDialog : public QDialog, private Ui::SplitRegionDialog {
	Q_OBJECT
public:
	SplitRegionDialog(QWidget* parent, VisualizationContainer* visualizationContainer);
	virtual ~SplitRegionDialog();
	
public slots:
	// Menu events
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).
	virtual void on_updateButton_clicked();

	virtual void on_reject();

protected:	
	VisualizationContainer* visualizationContainer;

	void SplitRegion();
};

#endif
