#ifndef RegionTable_H
#define RegionTable_H

#include <QTableWidget>

class Region;

class RegionTable : public QTableWidget {
  Q_OBJECT
public:
	RegionTable(QWidget* parent = 0);

	void Update(const std::vector<Region*>& regions);

protected:

};

#endif
