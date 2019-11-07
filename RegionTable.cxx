#include "RegionTable.h"

#include <QApplication>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QIcon>
#include <QStyle>
#include <QPushButton>
#include <QSignalMapper>
#include <QCheckBox>

#include "Region.h"

RegionTable::RegionTable(QWidget* parent)
	: QTableWidget(parent) 
{
	QStringList headers;
	headers << "Id" << "Color" << "Size" << "Done" << "Remove";
	setColumnCount(headers.length());
	setHorizontalHeaderLabels(headers);
	resizeColumnsToContents();
	verticalHeader()->setVisible(false);

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
		int label = (int)region->GetLabel();

		// Id
		QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(label));
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
		QCheckBox* checkBox = new QCheckBox(this);
		checkBox->setChecked(region->GetDone());
		checkBox->setStyleSheet("margin-left:10%;margin-right:5%;");
		QObject::connect(checkBox, &QCheckBox::stateChanged, [this, label](int state) {
			regionDone(label, state);
		});

		// Remove button
		QPushButton* removeButton = new QPushButton(this);
		removeButton->setIcon(removeIcon);
		QObject::connect(removeButton, &QPushButton::clicked, [this, label]() {
			removeRegion(label);
		});
		
		setItem(i, 0, idItem);
		setItem(i, 1, colorItem);
		setItem(i, 2, sizeItem);
		setCellWidget(i, 3, checkBox);
		setCellWidget(i, 4, removeButton);
	}

	resizeColumnsToContents();
}