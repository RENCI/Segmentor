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
#include <QColorDialog>

#include "Region.h"
#include "RegionCollection.h"

RegionTable::RegionTable(QWidget* parent)
	: QTableWidget(parent) 
{
	QStringList headers;
	headers << "Id" << "Color" << "Size" << "Refining" << "Visible" << "Done" << "Remove";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	enableSorting();
	setMouseTracking(true);
	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	regions = nullptr;

	currentRegionLabel = 0;

	showFeedbackColumns = false;

	QObject::connect(this, &RegionTable::removeRegion, this, &RegionTable::on_removeRegion);
	QObject::connect(this, &RegionTable::cellEntered, this, &RegionTable::on_cellEntered);
	QObject::connect(this, &RegionTable::cellClicked, this, &RegionTable::on_cellClicked);
}

void RegionTable::update() {
	if (!regions) return;

	disableSorting();

	int numRegions = regions->Size();
	setRowCount(numRegions);

	// Icon for remove
	QStyle* style = QApplication::style();
	QIcon removeIcon = style->standardIcon(QStyle::SP_DialogCloseButton);
	QIcon refiningIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);

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
		QTableWidgetItem* colorItem = new QTableWidgetItem();
		colorItem->setBackgroundColor(color);
		colorItem->setFlags(Qt::ItemIsSelectable);

		// Size
		QTableWidgetItem* sizeItem = new QTableWidgetItem();
		sizeItem->setData(0, region->GetNumVoxels());
		sizeItem->setTextAlignment(Qt::AlignCenter);
		sizeItem->setFlags(Qt::ItemIsSelectable);
		sizeItem->setTextColor(QColor("black"));

		// Refining
		bool refining = region->GetModified() && !region->GetDone();
		QTableWidgetItem* refiningItem = new QTableWidgetItem();
		refiningItem->setFlags(Qt::ItemIsSelectable);
		refiningItem->setData(0, refining);
		refiningItem->setTextColor(QColor("white"));

		// Using a button for the icon container sizes the icon correctly...
		QPushButton* refiningButton = new QPushButton();
		if (refining) refiningButton->setIcon(refiningIcon);
		refiningButton->setEnabled(false);
		refiningButton->setStyleSheet("background-color: transparent; border: none;");

		// Visible
		QTableWidgetItem* visibleItem = new QTableWidgetItem();
		visibleItem->setFlags(Qt::ItemIsSelectable);
		visibleItem->setData(0, region->GetVisible());
		visibleItem->setTextColor(QColor("white"));

		QCheckBox* visibleCheckbox = new QCheckBox();
		visibleCheckbox->setChecked(region->GetVisible());
		visibleCheckbox->setStyleSheet("margin-left:10%;margin-right:5%;");
		visibleCheckbox->setAttribute(Qt::WA_TransparentForMouseEvents);

		// Done
		QTableWidgetItem* doneItem = new QTableWidgetItem();
		doneItem->setFlags(Qt::ItemIsSelectable);
		doneItem->setData(0, region->GetDone());
		doneItem->setTextColor(QColor("white"));

		QCheckBox* doneCheckBox = new QCheckBox();
		doneCheckBox->setChecked(region->GetDone());
		doneCheckBox->setStyleSheet("margin-left:10%;margin-right:5%;");
		doneCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents);

		// Remove button
		QTableWidgetItem* removeItem = new QTableWidgetItem();
		removeItem->setFlags(Qt::ItemIsSelectable);

		QPushButton* removeButton = new QPushButton();
		removeButton->setIcon(removeIcon);
		removeButton->setEnabled(!region->GetDone());
		QObject::connect(removeButton, &QPushButton::clicked, [this, label]() {
			removeRegion(label);
		});

		setItem(i, Id, idItem);
		setItem(i, Color, colorItem);
		setItem(i, Size, sizeItem);

		setItem(i, Refining, refiningItem);
		setCellWidget(i, Refining, refiningButton);

		setItem(i, Visible, visibleItem);
		setCellWidget(i, Visible, visibleCheckbox);

		setItem(i, Done, doneItem);
		setCellWidget(i, Done, doneCheckBox);

		setItem(i, Remove, removeItem);
		setCellWidget(i, Remove, removeButton);
	}

	enableSorting();

	selectRegionLabel(currentRegionLabel);
}

void RegionTable::update(RegionCollection* regionCollection) {
	regions = regionCollection;

	update();
}

void RegionTable::update(Region* region) {
	disableSorting();

	QStyle* style = QApplication::style();
	QIcon refiningIcon = style->standardIcon(QStyle::SP_MessageBoxWarning);

	QString labelString = QString::number(region->GetLabel());

	for (int i = 0; i < rowCount(); i++) {
		QTableWidgetItem* ti = item(i, 0);

		if (ti->text() == labelString) {
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

void RegionTable::setShowFeedbackColumns(bool show) {
	showFeedbackColumns = show;

	update();
}

bool RegionTable::getShowFeedbackColumns() {
	return showFeedbackColumns;
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
}

void RegionTable::on_cellClicked(int row, int column) {
	disableSorting();

	if (column == Color) {
		QTableWidgetItem* colorItem = (QTableWidgetItem*)item(row, column);

		QColor color = QColorDialog::getColor(colorItem->backgroundColor());

		if (color.isValid()) {
			colorItem->setBackgroundColor(color);

			emit(regionColor(rowLabel(row), color));
		}
	}
	else if (column == Visible) {
		QCheckBox* checkBox = (QCheckBox*)cellWidget(row, column);
		checkBox->toggle();

		emit(regionVisible(rowLabel(row), checkBox->isChecked()));
	}
	else if (column == Done) {
		QCheckBox* checkBox = (QCheckBox*)cellWidget(row, column);
		checkBox->toggle();

		QPushButton* button = (QPushButton*)cellWidget(row, Remove);
		button->setEnabled(!checkBox->isChecked());

		emit(regionDone(rowLabel(row), checkBox->isChecked()));
	}
	else {
		emit(selectRegion(rowLabel(row)));
	}

	enableSorting();
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

void RegionTable::disableSorting() {
	QObject::disconnect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &QTableWidget::resizeColumnsToContents);
	setSortingEnabled(false);
}

void RegionTable::enableSorting() {
	setSortingEnabled(true);
	QObject::connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &QTableWidget::resizeColumnsToContents);
	resizeColumnsToContents();
}