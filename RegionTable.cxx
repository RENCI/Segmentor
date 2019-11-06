#include "RegionTable.h"

#include <QApplication>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QIcon>
#include <QStyle>
#include <QPushButton>

#include "Region.h"

RegionTable::RegionTable(QWidget* parent)
	: QTableWidget(parent) 
{
	QStringList headers;
	headers << "Id" << "Color" << "Size" << "Done" << "Remove";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);

	horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
}

void RegionTable::Update(const std::vector<Region*>& regions) {
	int numRegions = (int)regions.size();

	setRowCount(numRegions);

	// Icon for remove
	QStyle* style = QApplication::style();
	QIcon removeIcon = style->standardIcon(QStyle::SP_DialogCancelButton);

	for (int i = 0; i < numRegions; i++) {
		Region* region = regions[i];

		// Id
		QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(region->GetLabel()));
		idItem->setTextAlignment(Qt::AlignCenter);

		// Color
		const double* col = region->GetColor();
		QColor color(col[0] * 255, col[1] * 255, col[2] * 255);
		QTableWidgetItem* colorItem = new QTableWidgetItem("");
		colorItem->setBackgroundColor(color);

		// Size
		QTableWidgetItem* sizeItem = new QTableWidgetItem(QString::number(region->GetNumVoxels()));
		sizeItem->setTextAlignment(Qt::AlignCenter);

		// Checkbox
		//QCheckbox* checkBox = new QCheckBox();
		QTableWidgetItem* checkItem = new QTableWidgetItem();
		checkItem->setCheckState(Qt::Unchecked);

		// Remove button
		QPushButton* removeButton = new QPushButton(this);
		removeButton->setIcon(removeIcon);
		
		setItem(i, 0, idItem);
		setItem(i, 1, colorItem);
		setItem(i, 2, sizeItem);
		setItem(i, 3, checkItem);
		setCellWidget(i, 4, removeButton);
	}
}