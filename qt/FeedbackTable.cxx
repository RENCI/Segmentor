#include "FeedbackTable.h"

#include <QApplication>
#include <QCheckBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QStyle>
#include <QSignalMapper>

#include "Region.h"
#include "RegionCollection.h"
#include "LineEditDelegate.h"

FeedbackTable::FeedbackTable(QWidget* parent) : QTableWidget(parent) {
	QStringList headers;
	headers << "Id" << "Comment" << "Done" << "Verified";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	enableSorting();
	setMouseTracking(true);

	LineEditDelegate* lineEdit = new LineEditDelegate(this);
	setItemDelegateForColumn(1, lineEdit);

	regions = nullptr;

	currentRegionLabel = 0;

	filter = false;

	QObject::connect(this, &FeedbackTable::cellEntered, this, &FeedbackTable::on_cellEntered);
	QObject::connect(this, &FeedbackTable::cellClicked, this, &FeedbackTable::on_cellClicked);
	QObject::connect(this, &FeedbackTable::cellChanged, this, &FeedbackTable::on_cellChanged);
}

void FeedbackTable::update() {
	if (!regions) return;

	disableSorting();

	std::vector<Region*> displayRegions;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);

		if (!filter || region->HasComment()) {
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
		commentItem->setData(0, QString::fromStdString(region->GetComment()));
		setItem(i, Comment, commentItem);

		// Done
		addCheckWidget(i, Done, region->GetDone(), !region->GetVerified());

		// Verified
		addCheckWidget(i, Verified, region->GetVerified(), region->GetDone());
	}

	enableSorting();

	selectRegionLabel(currentRegionLabel);

	emit(countChanged(numRegions));

	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
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
	if (column == Id) {
		emit(selectRegion(rowLabel(row)));
	}
	else if (column == Done) {
		QCheckBox* checkBox = (QCheckBox*)cellWidget(row, column);
		checkBox->toggle();

		bool done = checkBox->isChecked();

		QCheckBox* verifiedCheckBox = (QCheckBox*)cellWidget(row, Verified);
		verifiedCheckBox->setEnabled(done);
		
		emit(regionDone(rowLabel(row), checkBox->isChecked()));
	}
	else if (column == Verified) {
		QCheckBox* checkBox = (QCheckBox*)cellWidget(row, column);
		checkBox->toggle();

		emit(regionVerified(rowLabel(row), checkBox->isChecked()));
	}
}

void FeedbackTable::on_cellChanged(int row, int column) {
	if (column == Comment) {
		QTableWidgetItem* ti = item(row, Comment);

		emit(regionComment(rowLabel(row), ti->data(0).toString()));
	}
}

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


void FeedbackTable::addCheckWidget(int row, int column, bool checked, bool enabled) {
	QTableWidgetItem* item = new QTableWidgetItem();
	item->setFlags(Qt::ItemIsSelectable);
	item->setTextColor(QColor("white"));

	QCheckBox* checkBox = new QCheckBox();
	checkBox->setChecked(checked);
	checkBox->setEnabled(enabled);
	checkBox->setStyleSheet("margin-left:15%;margin-right:10%;padding-left:0;padding-right:0");
	checkBox->setAttribute(Qt::WA_TransparentForMouseEvents);

	setItem(row, column, item);
	setCellWidget(row, column, checkBox);
}
