#ifndef RegionCollection_H
#define RegionCollection_H

#include <map>

class Region;

class RegionCollection {
protected:
	typedef std::map<unsigned short, Region*> CollectionType;

public:
	RegionCollection();
	~RegionCollection();

	// Basic management
	bool Add(Region* region);
	bool Has(unsigned short label);
	Region* Get(unsigned short label);

	void Remove(unsigned short label);
	void RemoveAll();

	int Size();

	unsigned short GetNewLabel();

	// Traversal
	typedef CollectionType::iterator Iterator;
	Iterator Begin();
	Iterator End();
	Region* Get(Iterator iterator);

protected:
	CollectionType regions;
};

#endif
