#include "FeedbackTable.h"

#include <QApplication>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QPushButton>
#include <QSignalMapper>
#include <QCheckBox>
#include <QColorDialog>

#include "Feedback.h"
#include "LabelColors.h"
#include "Region.h"
#include "RegionCollection.h"

FeedbackTable::FeedbackTable(QWidget* parent) : QTableWidget(parent) {
	QStringList headers;
	headers << "Id" << "Undertraced" << "Overtraced" << "Add to Slice" << "Remove Id" << "Split" << "Merge" << "Correct from Split/Merge";
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
	QObject::connect(this, &FeedbackTable::cellClicked, this, &FeedbackTable::on_cellClicked);
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

		// Check boxes
		addCheckWidget(i, Undertraced, region->GetFeedback()->GetValue(Feedback::Undertraced));
		addCheckWidget(i, Overtraced, region->GetFeedback()->GetValue(Feedback::Overtraced));
		addCheckWidget(i, AddToSlice, region->GetFeedback()->GetValue(Feedback::AddToSlice));
		addCheckWidget(i, RemoveId, region->GetFeedback()->GetValue(Feedback::RemoveId));
		addCheckWidget(i, Split, region->GetFeedback()->GetValue(Feedback::Split));
		addCheckWidget(i, Merge, region->GetFeedback()->GetValue(Feedback::Merge));
		addCheckWidget(i, CorrectSplitMerge, region->GetFeedback()->GetValue(Feedback::CorrectSplitMerge));
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
	if (column == Id) return;

	disableSorting();

	QCheckBox * checkBox = (QCheckBox*)cellWidget(row, column);
	checkBox->toggle();

	emit(regionFeedback(rowLabel(row), columnToFeedback(column), checkBox->isChecked()));

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

void FeedbackTable::addCheckWidget(int row, int column, bool checked) {
	QTableWidgetItem* item = new QTableWidgetItem();
	item->setFlags(Qt::ItemIsSelectable);
	item->setData(0, checked);
	item->setTextColor(QColor("white"));

	QCheckBox* checkBox = new QCheckBox();
	checkBox->setChecked(checked);
	checkBox->setStyleSheet("margin-left:auto;margin-right:auto;");
	checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);

	setItem(row, column, item);
	setCellWidget(row, column, checkBox);
}

Feedback::FeedbackType FeedbackTable::columnToFeedback(int column) {
	switch (column) {
	case Undertraced: return Feedback::Undertraced;
	case Overtraced: return Feedback::Overtraced;
	case AddToSlice: return Feedback::AddToSlice;
	case RemoveId: return Feedback::RemoveId;
	case Split: return Feedback::Split;
	case Merge: return Feedback::Merge;
	case CorrectSplitMerge: return Feedback::CorrectSplitMerge;
	default: return Feedback::Undertraced;
	}
}