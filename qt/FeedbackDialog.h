#ifndef FeedbackDialog_H
#define FeedbackDialog_H

#include "ui_FeedbackDialog.h"

#include "Feedback.h"

class FeedbackTable;
class VisualizationContainer;

class FeedbackDialog : public QDialog, private Ui::FeedbackDialog {
	Q_OBJECT
public:
	FeedbackDialog(QWidget* parent, VisualizationContainer* visualizationContainer);
	virtual ~FeedbackDialog();

	void updateRegions();
	
public slots:
	// Use Qt's auto-connect magic to tie GUI widgets to slots,
	// removing the need to call connect() explicitly.
	// Names of the methods must follow the naming convention
	// on_<widget name>_<signal name>(<signal parameters>).
	void on_filterCheckBox_stateChanged(int state);

	void on_regionFeedback(int label, Feedback::FeedbackType type, bool value);
	void on_highlightRegion(int label);
	void on_countChanged(int count);

protected:
	FeedbackTable* table;
	VisualizationContainer* visualizationContainer;
};

#endif
