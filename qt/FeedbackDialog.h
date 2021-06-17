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
	void on_regionFeedback(int label, Feedback::FeedbackType type, bool value);
	void on_highlightRegion(int label);

protected:
	FeedbackTable* table;
	VisualizationContainer* visualizationContainer;
};

#endif
