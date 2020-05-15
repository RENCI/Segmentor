#ifndef RegionTable_H
#define RegionTable_H

#include <QTableWidget>

class Region;
class RegionCollection;

class RegionTable : public QTableWidget {
  Q_OBJECT
public:
	RegionTable(QWidget* parent = 0);

	void update(RegionCollection* regions);
	void update(Region* region);
	void selectRegionLabel(unsigned short label);

public slots:
	void on_removeRegion(int label);
	void on_cellEntered(int row, int column);
	void on_cellClicked(int row, int column);

signals:
	void regionDone(int label, bool done);
	void removeRegion(int label);
	void highlightRegion(int label);
	void selectRegion(int label);

protected:
	int currentRegionLabel;

	void leaveEvent(QEvent *event);

	int rowLabel(int row);

	void disableSorting();
	void enableSorting();
};

#endif
