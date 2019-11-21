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
#include "RegionCollection.h"

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

void RegionTable::Update(RegionCollection* regions) {
	int numRegions = regions->Size();

	setRowCount(numRegions);

	// Icon for remove
	QStyle* style = QApplication::style();
	QIcon removeIcon = style->standardIcon(QStyle::SP_DialogCancelButton);

	// Grey color
	double grey[3] = { 0.5, 0.5, 0.5 };

	int i = 0;
	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++, i++) {
		Region* region = regions->Get(it);
		int label = (int)region->GetLabel();

		// Id
		QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(label));
		idItem->setTextAlignment(Qt::AlignCenter);
		idItem->setFlags(Qt::ItemIsSelectable);

		// Color
		const double* col = region->GetDone() ? grey : region->GetColor();
		QColor color(col[0] * 255, col[1] * 255, col[2] * 255);
		QTableWidgetItem* colorItem = new QTableWidgetItem("");
		colorItem->setBackgroundColor(color);
		colorItem->setFlags(Qt::ItemIsSelectable);

		// Size
		QTableWidgetItem* sizeItem = new QTableWidgetItem(QString::number(region->GetNumVoxels()));
		sizeItem->setTextAlignment(Qt::AlignCenter);
		sizeItem->setFlags(Qt::ItemIsSelectable);
		sizeItem->setTextColor(QColor("black"));

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

	Highlight(0);
}

void RegionTable::Highlight(unsigned short label) {
	QString labelString = QString::number(label);

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