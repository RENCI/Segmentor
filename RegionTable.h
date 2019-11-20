#ifndef RegionTable_H
#define RegionTable_H

#include <QTableWidget>

class QSignalMapper;

class RegionCollection;

class RegionTable : public QTableWidget {
  Q_OBJECT
public:
	RegionTable(QWidget* parent = 0);

	void Update(RegionCollection* regions);
	void Highlight(unsigned short label);

signals:
	void regionDone(int label, bool done);
	void removeRegion(int label);
};

#endif
