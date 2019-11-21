#ifndef RegionTable_H
#define RegionTable_H

#include <QTableWidget>

class QSignalMapper;

class RegionCollection;

class RegionTable : public QTableWidget {
  Q_OBJECT
public:
	RegionTable(QWidget* parent = 0);

	void update(RegionCollection* regions);
	void highlight(unsigned short label);

public slots:
	void on_sort();

signals:
	void regionDone(int label, bool done);
	void removeRegion(int label);
};

#endif
