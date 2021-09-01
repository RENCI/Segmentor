#ifndef RegionTable_H
#define RegionTable_H

#include <QTableWidget>
#include <QColor>

class Region;
class RegionCollection;

class RegionTable : public QTableWidget {
  Q_OBJECT
public:
	RegionTable(QWidget* parent = 0);

	void update();
	void update(RegionCollection* regionCollection);
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
	void regionVisible(int label, bool visible);
	void regionColor(int label, QColor color);

protected:
	RegionCollection* regions;

	int currentRegionLabel;

	void leaveEvent(QEvent *event);

	int rowLabel(int row);

	void disableSorting();
	void enableSorting();

	enum ColumnType {
		Id = 0,
		Color,
		Size,
		Refining,
		Visible,
		Done,
		Remove
	};

	void addCheckWidget(int row, int column, bool checked);
};

#endif
