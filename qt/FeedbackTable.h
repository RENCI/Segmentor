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
	void update(Region* region);
	void selectRegionLabel(unsigned short label);

	void setFilter(bool filterRows);

public slots:
	void on_cellEntered(int row, int column);
	void on_cellClicked(int row, int column);
	void on_cellChanged(int row, int column);
	void on_sortingChanged();

signals:
	void selectRegion(int label);
	void regionComment(int label, QString comment);
	void regionDone(int label, bool done);
	void regionVerified(int label, bool verified);
	void highlightRegion(int label);
	void countChanged(int count);

protected:
	RegionCollection * regions;

	int currentRegionLabel;

	bool filter;

	void leaveEvent(QEvent *event);

	int rowLabel(int row);

	void disableSorting();
	void enableSorting();
	void setColumnSizes();

	enum ColumnType {
		Id = 0,
		Comment,
		Done,
		Verified
	};

	void addCheckWidget(int row, int column, bool checked, bool enabled = true);
};

#endif
