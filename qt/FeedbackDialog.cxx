#include "FeedbackDialog.h"

#include "FeedbackTable.h"
#include "VisualizationContainer.h"

#include <QDialogButtonBox>
#include <QPushButton>

// Constructor
FeedbackDialog::FeedbackDialog(QWidget* parent, VisualizationContainer* visualizationContainer)
: QDialog(parent), visualizationContainer(visualizationContainer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);


	buttonBox->button(QDialogButtonBox::Close)->setAutoDefault(false);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);	

	// Create table
	table = new FeedbackTable(this);
	table->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	table->update(visualizationContainer->GetRegions());
	tableContainer->layout()->addWidget(table);
	
	QObject::connect(table, &FeedbackTable::regionComment, this, &FeedbackDialog::on_regionComment);
	QObject::connect(table, &FeedbackTable::highlightRegion, this, &FeedbackDialog::on_highlightRegion);
	QObject::connect(table, &FeedbackTable::countChanged, this, &FeedbackDialog::on_countChanged);

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

void FeedbackDialog::on_regionComment(int label, QString comment) {
	visualizationContainer->SetRegionComment(label, comment.toStdString());
}

void FeedbackDialog::on_highlightRegion(int label) {
	visualizationContainer->HighlightRegion((unsigned short)label);
}

void FeedbackDialog::on_countChanged(int count) {
	countLabel->setText("Count: " + QString::number(count));
}