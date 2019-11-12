#ifndef RegionCollection_H
#define RegionCollection_H

#include <map>

#include <vtkSmartPointer.h>

class Region;

class RegionCollection {
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

	// Traversal
	void InitTraversal();
	Region* GetNext();

protected:
	std::map<unsigned short, Region*> regions;

	std::map<unsigned short, Region*>::iterator it;
};

#endif
