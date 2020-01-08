#include "RegionTable.h"

#include <QApplication>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QIcon>
#include <QLabel>
#include <QStyle>
#include <QPushButton>
#include <QSignalMapper>
#include <QCheckBox>

#include "Region.h"
#include "RegionCollection.h"

RegionTable::RegionTable(QWidget* parent)
	: QTableWidget(parent) 
{
	QStringList headers;
	headers << "Id" << "Color" << "Size" << "Modified" << "Done" << "Remove";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	setSortingEnabled(true);
	resizeColumnsToContents();
	setMouseTracking(true);
	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	currentRegionLabel = 0;

	QObject::connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &RegionTable::on_sort);
	QObject::connect(this, &RegionTable::removeRegion, this, &RegionTable::on_removeRegion);
	QObject::connect(this, &RegionTable::cellEntered, this, &RegionTable::on_cellEntered);
	QObject::connect(this, &RegionTable::cellClicked, this, &RegionTable::on_cellClicked);
}

void RegionTable::update(RegionCollection* regions) {
	int numRegions = regions->Size();

	// Need to disable sorting 
	setSortingEnabled(false);

	// Clear table
	setRowCount(numRegions);

	// Icon for remove
	QStyle* style = QApplication::style();
	QIcon removeIcon = style->standardIcon(QStyle::SP_DialogCloseButton);
	QIcon modifiedIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);

	// Grey color
	double grey[3] = { 0.5, 0.5, 0.5 };

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

		// Color
		const double* col = region->GetDone() ? grey : region->GetColor();
		QColor color(col[0] * 255, col[1] * 255, col[2] * 255);
		QTableWidgetItem* colorItem = new QTableWidgetItem("");
		colorItem->setBackgroundColor(color);
		colorItem->setFlags(Qt::ItemIsSelectable);

		// Size
		QTableWidgetItem* sizeItem = new QTableWidgetItem();
		sizeItem->setData(0, region->GetNumVoxels());
		sizeItem->setTextAlignment(Qt::AlignCenter);
		sizeItem->setFlags(Qt::ItemIsSelectable);
		sizeItem->setTextColor(QColor("black"));

		// Modified
		QTableWidgetItem* modifiedItem = new QTableWidgetItem();
		modifiedItem->setFlags(Qt::ItemIsSelectable);
		modifiedItem->setData(0, region->GetModified());
		modifiedItem->setTextColor(QColor("white"));

		// Using a button for the icon container sizes the icon correctly...
		QPushButton* modifiedButton = new QPushButton();
		if (region->GetModified()) modifiedButton->setIcon(modifiedIcon);
		modifiedButton->setEnabled(false);
		modifiedButton->setStyleSheet("background-color: transparent; border: none;");

		// Done
		QTableWidgetItem* doneItem = new QTableWidgetItem();
		doneItem->setFlags(Qt::ItemIsSelectable);
		doneItem->setData(0, region->GetDone());
		doneItem->setTextColor(QColor("white"));

		QCheckBox* doneCheckBox = new QCheckBox();
		doneCheckBox->setChecked(region->GetDone());
		doneCheckBox->setStyleSheet("margin-left:10%;margin-right:5%;");
		QObject::connect(doneCheckBox, &QCheckBox::stateChanged, [this, label](int state) {
			regionDone(label, state);
		});

		// Remove button
		QTableWidgetItem* removeItem = new QTableWidgetItem();
		removeItem->setFlags(Qt::ItemIsSelectable);

		QPushButton* removeButton = new QPushButton();
		removeButton->setIcon(removeIcon);
		QObject::connect(removeButton, &QPushButton::clicked, [this, label]() {
			removeRegion(label);
		});
		
		setItem(i, 0, idItem);
		setItem(i, 1, colorItem);
		setItem(i, 2, sizeItem);

		setItem(i, 3, modifiedItem);
		setCellWidget(i, 3, modifiedButton);

		setItem(i, 4, doneItem);
		setCellWidget(i, 4, doneCheckBox);

		setItem(i, 5, removeItem);
		setCellWidget(i, 5, removeButton);
	}

	// Enable sorting
	setSortingEnabled(true);

	// Resize columns
	resizeColumnsToContents();

	// Need to reconnect after disabling and enabling sorting
	QObject::connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &RegionTable::on_sort);

	selectRegionLabel(currentRegionLabel);
}

void RegionTable::update(Region* region) {
	QStyle* style = QApplication::style();
	QIcon modifiedIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);

	QString labelString = QString::number(region->GetLabel());

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) {
			// Size
			item(i, 2)->setData(0, region->GetNumVoxels());

			// Modified
			item(i, 3)->setData(0, region->GetModified());
			((QPushButton*)cellWidget(i, 3))->setIcon(region->GetModified() ? modifiedIcon : QIcon());

			// Done
			item(i, 4)->setData(0, region->GetDone());
			((QCheckBox*)cellWidget(i, 4))->setChecked(region->GetDone());

			break;
		}
	}
}

void RegionTable::selectRegionLabel(unsigned short label) {
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

void RegionTable::on_sort() {
	resizeColumnsToContents();
}

void RegionTable::on_removeRegion(int label) {
	QString labelString = QString::number(label);

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) {
			removeRow(i);

			break;
		}
	}
}

void RegionTable::on_cellEntered(int row, int column) {
	QString labelString = QString::number(currentRegionLabel);

	if (column <= 2) {
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

void RegionTable::on_cellClicked(int row, int column) {
	if (column <= 2) {
		emit(selectRegion(rowLabel(row)));
	}
}

void RegionTable::leaveEvent(QEvent* event) {
	// Clear highlight
	QString labelString = QString::number(currentRegionLabel);

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) continue;

		ti->setBackgroundColor(QColor("white"));
	}

	emit(highlightRegion(0));
}

int RegionTable::rowLabel(int row) {
	return item(row, 0)->text().toInt();
}