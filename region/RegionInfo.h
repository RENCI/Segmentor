#ifndef RegionInfo_H
#define RegionInfo_H

#include <vtkSmartPointer.h>

class Region;
class RegionMetadataIO;

class RegionInfo {
public:
	RegionInfo();
	RegionInfo(Region* region);
	~RegionInfo();

protected:
	unsigned short label;
	double color[3];
	int extent[6];

	bool visible;
	bool modified;
	bool done;
	bool verified;
	std::string comment;

	friend class Region;
	friend class RegionMetadataIO;
};

#endif
