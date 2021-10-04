#include "FeedbackDialog.h"

#include "FeedbackTable.h"
#include "Region.h"
#include "RegionCollection.h"
#include "VisualizationContainer.h"

#include <QCompleter>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>
#include <QStandardItemModel>

// Constructor
FeedbackDialog::FeedbackDialog(QWidget* parent, VisualizationContainer* visualizationContainer)
: QDialog(parent), visualizationContainer(visualizationContainer) {
	// Create the GUI from the Qt Designer file
	setupUi(this);

	buttonBox->button(QDialogButtonBox::Close)->setAutoDefault(false);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);	

	// Search autocomplete
	labelModel = new QStandardItemModel(this);
	QCompleter* completer = new QCompleter(labelModel, this);
	searchLineEdit->setCompleter(completer);

	// Create table
	table = new FeedbackTable(this);
	table->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	table->update(visualizationContainer->GetRegions());
	tableContainer->layout()->addWidget(table);
	
	QObject::connect(table, &FeedbackTable::regionComment, this, &FeedbackDialog::on_regionComment);
	QObject::connect(table, &FeedbackTable::regionDone, this, &FeedbackDialog::on_regionDone);
	QObject::connect(table, &FeedbackTable::regionVerified, this, &FeedbackDialog::on_regionVerified);
	QObject::connect(table, &FeedbackTable::selectRegion, this, &FeedbackDialog::on_selectRegion);
	QObject::connect(table, &FeedbackTable::highlightRegion, this, &FeedbackDialog::on_highlightRegion);
	QObject::connect(table, &FeedbackTable::countChanged, this, &FeedbackDialog::on_countChanged);

	// Shortcuts
	QShortcut* shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_V), this);
	QObject::connect(shortcut, &QShortcut::activated, this, &FeedbackDialog::on_verifiedShortcut);

	QShortcut* closeShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this);
	QObject::connect(closeShortcut, &QShortcut::activated, this, &FeedbackDialog::close);

	updateRegions();
}

FeedbackDialog::~FeedbackDialog() {
	if (table) delete table;
}

void FeedbackDialog::updateRegions() {
	RegionCollection* regions = visualizationContainer->GetRegions();

	// Update table
	table->update(regions);

	// Update autocomplete
	labelModel->clear();
	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);

		QStandardItem* item = new QStandardItem(QString::number(region->GetLabel()));

		labelModel->appendRow(item);
	}
}

void FeedbackDialog::updateRegion(Region* region) {
	table->update(region);
}

void FeedbackDialog::selectRegionLabel(unsigned short label) {
	table->selectRegionLabel(label);
}

void FeedbackDialog::on_searchLineEdit_editingFinished() {
	unsigned short label = searchLineEdit->text().toInt();

	visualizationContainer->SelectRegion(label);
}

void FeedbackDialog::on_filterCheckBox_stateChanged(int state) {
	table->setFilter(state != 0);
}

void FeedbackDialog::on_regionComment(int label, QString comment) {
	visualizationContainer->SetRegionComment(label, comment.toStdString());
}

void FeedbackDialog::on_regionDone(int label, bool done) {
	visualizationContainer->SetRegionDone(label, done);
}

void FeedbackDialog::on_regionVerified(int label, bool verified) {
	visualizationContainer->SetRegionVerified(label, verified);
}

void FeedbackDialog::on_selectRegion(int label) {
	visualizationContainer->SelectRegion((unsigned short)label);
}

void FeedbackDialog::on_highlightRegion(int label) {
	visualizationContainer->HighlightRegion((unsigned short)label);
}

void FeedbackDialog::on_countChanged(int count) {
	countLabel->setText("Count: " + QString::number(count));
}

void FeedbackDialog::on_verifiedShortcut() {
	printf("DLKJF");
}