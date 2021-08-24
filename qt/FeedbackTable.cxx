#include "FeedbackTable.h"

#include <QApplication>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QStyle>
#include <QSignalMapper>
#include <QCheckBox>
#include <QTextEdit>

#include "Feedback.h"
#include "Region.h"
#include "RegionCollection.h"

FeedbackTable::FeedbackTable(QWidget* parent) : QTableWidget(parent) {
	QStringList headers;
	headers << "Id" << "Comment" << "Status";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	enableSorting();
	setMouseTracking(true);
	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	regions = nullptr;

	currentRegionLabel = 0;

	filter = false;

	QObject::connect(this, &FeedbackTable::cellEntered, this, &FeedbackTable::on_cellEntered);
//	QObject::connect(this, &FeedbackTable::cellClicked, this, &FeedbackTable::on_cellClicked);
}

void FeedbackTable::update() {
	if (!regions) return;

	disableSorting();

	std::vector<Region*> displayRegions;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);

		if (!filter || region->GetFeedback()->HasFeedback()) {
			displayRegions.push_back(region);
		}
	}
	
	int numRegions = (int)displayRegions.size();
	setRowCount(numRegions);

	// Add rows for each region
	for (int i = 0; i < numRegions; i++) {
		Region* region = displayRegions[i];
		int label = (int)region->GetLabel();

		// Id
		QTableWidgetItem* idItem = new QTableWidgetItem();
		idItem->setData(0, label);
		idItem->setTextAlignment(Qt::AlignCenter);
		idItem->setFlags(Qt::ItemIsSelectable);

		setItem(i, Id, idItem);

		// Comment
		QTableWidgetItem* commentItem = new QTableWidgetItem();
		commentItem->setFlags(Qt::ItemIsSelectable);
		commentItem->setData(0, "comment");

		QTextEdit* edit = new QTextEdit();
		edit->setText("");
		edit->setAttribute(Qt::WA_TransparentForMouseEvents);

		setItem(i, Comment, commentItem);
		setCellWidget(i, Comment, edit);

		// Status
		QTableWidgetItem* statusItem = new QTableWidgetItem();
		statusItem->setFlags(Qt::ItemIsSelectable);
		statusItem->setData(0, true);
		statusItem->setTextColor(QColor("white"));

		QCheckBox* checkBox = new QCheckBox();
		checkBox->setChecked(true);
		checkBox->setStyleSheet("margin-left:auto;margin-right:auto;");
		checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);

		setItem(i, Status, statusItem);
		setCellWidget(i, Status, checkBox);
	}

	enableSorting();

	selectRegionLabel(currentRegionLabel);

	emit(countChanged(numRegions));
}

void FeedbackTable::update(RegionCollection* regionCollection) {
	regions = regionCollection;

	update();
}

void FeedbackTable::selectRegionLabel(unsigned short label) {
	currentRegionLabel = label;
	QString labelString = QString::number(currentRegionLabel);

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) {
			ti->setBackgroundColor(QColor("#1d91c0"));
			ti->setTextColor(QColor("white"));

			scrollToItem(ti);
		}
		else {
			ti->setBackgroundColor(QColor("white"));
			ti->setTextColor(QColor("black"));
		}
	}
}

void FeedbackTable::setFilter(bool filterRows) {
	filter = filterRows;

	update();
}

void FeedbackTable::on_cellEntered(int row, int column) {
	QString labelString = QString::number(currentRegionLabel);

	if (column == Id) {
		// Highlight
		for (int i = 0; i < rowCount(); i++) {
			QTableWidgetItem* ti = item(i, 0);

			if (ti->text() == labelString) continue;

			if (i == row) {
				ti->setBackgroundColor(QColor("#bfe6f5"));
			}
			else {
				ti->setBackgroundColor(QColor("white"));
			}
		}

		emit(highlightRegion(rowLabel(row)));
	}
	else {
		// Clear highlight
		for (int i = 0; i < rowCount(); i++) {
			QTableWidgetItem* ti = item(i, 0);

			if (ti->text() == labelString) continue;

			ti->setBackgroundColor(QColor("white"));
		}

		emit(highlightRegion(0));
	}
}


void FeedbackTable::on_cellClicked(int row, int column) {
	if (column != Status) return;

	disableSorting();

	QCheckBox * checkBox = (QCheckBox*)cellWidget(row, column);
	checkBox->toggle();

	emit(regionFeedback(rowLabel(row), Feedback::Status, checkBox->isChecked()));

	enableSorting();
}


/*
void FeedbackTable::leaveEvent(QEvent* event) {
	// Clear highlight
	QString labelString = QString::number(currentRegionLabel);

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) continue;

		ti->setBackgroundColor(QColor("white"));
	}

	emit(highlightRegion(0));
}
*/

int FeedbackTable::rowLabel(int row) {
	return item(row, 0)->text().toInt();
}

void FeedbackTable::disableSorting() {
	QObject::disconnect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &QTableWidget::resizeColumnsToContents);
	setSortingEnabled(false);
}

void FeedbackTable::enableSorting() {
	setSortingEnabled(true);
	QObject::connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &QTableWidget::resizeColumnsToContents);
	resizeColumnsToContents();
}