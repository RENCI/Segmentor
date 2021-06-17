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
	headers << "Id" << "Undertraced" << "Overtraced" << "Add to Slice" << "Remove Id" << "Correct from Split/Merge";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	enableSorting();
	setMouseTracking(true);
	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	currentRegionLabel = 0;

	QObject::connect(this, &FeedbackTable::cellClicked, this, &FeedbackTable::on_cellClicked);
}

void FeedbackTable::update(RegionCollection* regions) {
	if (!regions) return;

	disableSorting();

	int numRegions = regions->Size();
	setRowCount(numRegions);

	// Add rows for each region
	int i = 0;
	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++, i++) {
		Region* region = regions->Get(it);
		int label = (int)region->GetLabel();

		// Id
		QTableWidgetItem* idItem = new QTableWidgetItem();
		idItem->setData(0, label);
		idItem->setTextAlignment(Qt::AlignCenter);
		idItem->setFlags(Qt::ItemIsSelectable);

		setItem(i, Id, idItem);

		// Check boxes
		addCheckWidget(i, Undertraced, false);
		addCheckWidget(i, Overtraced, false);
		addCheckWidget(i, AddToSlice, false);
		addCheckWidget(i, RemoveId, false);
		addCheckWidget(i, CorrectSplitMerge, false);
	}

	enableSorting();

	selectRegionLabel(currentRegionLabel);
}

void FeedbackTable::update(Region* region) {
/*
	disableSorting();

	QStyle* style = QApplication::style();
	QIcon refiningIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);

	QString labelString = QString::number(region->GetLabel());

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) {
			// Color
			const double* col = region->GetDone() ? LabelColors::doneColor : region->GetColor();
			QColor color(col[0] * 255, col[1] * 255, col[2] * 255);
			item(i, Color)->setBackgroundColor(color);

			// Size
			item(i, Size)->setData(0, region->GetNumVoxels());

			// Refining
			bool refining = region->GetModified() && !region->GetDone();
			item(i, Refining)->setData(0, refining);
			((QPushButton*)cellWidget(i, Refining))->setIcon(refining ? refiningIcon : QIcon());

			// Visible
			item(i, Visible)->setData(0, region->GetVisible());
			((QCheckBox*)cellWidget(i, Visible))->setChecked(region->GetVisible());

			// Done
			item(i, Done)->setData(0, region->GetDone());
			((QCheckBox*)cellWidget(i, Done))->setChecked(region->GetDone());

			// Remove
			((QPushButton*)cellWidget(i, Remove))->setEnabled(!region->GetDone());

			break;
		}
	}

	enableSorting();
*/
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

void FeedbackTable::on_cellEntered(int row, int column) {
/*
	QString labelString = QString::number(currentRegionLabel);

	if (column == Id || column == Color || column == Size) {
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
*/
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
	checkBox->setStyleSheet("margin-left:10%;margin-right:5%;");
	checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);

	setItem(row, column, item);
	setCellWidget(row, column, checkBox);
}

Feedback::FeedbackType FeedbackTable::columnToFeedback(int column) {
	switch (column) {
	case Feedback::Undertraced: return Feedback::Undertraced;
	case Feedback::Overtraced: return Feedback::Overtraced;
	case Feedback::AddToSlice: return Feedback::AddToSlice;
	case Feedback::RemoveId: return Feedback::RemoveId;
	case Feedback::CorrectSplitMerge: return Feedback::CorrectSplitMerge;
	default: return Feedback::Undertraced;
	}
}