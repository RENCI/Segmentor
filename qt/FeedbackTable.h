#ifndef FeedbackTable_H
#define FeedbackTable_H

#include <QTableWidget>
#include <QColor>

class Region;
class RegionCollection;

class FeedbackTable : public QTableWidget {
  Q_OBJECT
public:
	FeedbackTable(QWidget* parent = 0);	

	void update();
	void update(RegionCollection* regionCollection);
	void selectRegionLabel(unsigned short label);

	void setFilter(bool filterRows);

public slots:
	void on_cellEntered(int row, int column);
	void on_cellClicked(int row, int column);

signals:
	void regionComment(int label, QString comment);
	void highlightRegion(int label);
	void countChanged(int count);

protected:
	RegionCollection * regions;

	int currentRegionLabel;

	bool filter;

	int rowLabel(int row);

	void disableSorting();
	void enableSorting();

	enum ColumnType {
		Id = 0,
		Comment,
		Status
	};
};

#endif
