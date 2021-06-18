#include "FeedbackDialog.h"

#include "Feedback.h"
#include "FeedbackTable.h"
#include "VisualizationContainer.h"

// Constructor
FeedbackDialog::FeedbackDialog(QWidget* parent, VisualizationContainer* visualizationContainer)
: QDialog(parent), visualizationContainer(visualizationContainer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);	

	// Create table
	table = new FeedbackTable(this);
	table->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	table->update(visualizationContainer->GetRegions());
	tableContainer->layout()->addWidget(table);
	
	QObject::connect(table, &FeedbackTable::regionFeedback, this, &FeedbackDialog::on_regionFeedback);
	QObject::connect(table, &FeedbackTable::highlightRegion, this, &FeedbackDialog::on_highlightRegion);

	updateRegions();
}

FeedbackDialog::~FeedbackDialog() {
	if (table) delete table;
}

void FeedbackDialog::updateRegions() {
	table->update(visualizationContainer->GetRegions());
}

void FeedbackDialog::on_filterCheckBox_stateChanged(int state) {
	printf("%d\n", state);

	table->setFilter(state != 0);
}

void FeedbackDialog::on_regionFeedback(int label, Feedback::FeedbackType type, bool value) {
	visualizationContainer->SetRegionFeedback(label, type, value);
}

void FeedbackDialog::on_highlightRegion(int label) {
	visualizationContainer->HighlightRegion((unsigned short)label);
}