#include "RegionCollection.h"

#include "Region.h"

RegionCollection::RegionCollection() {
}
	
RegionCollection::~RegionCollection() {
	RemoveAll();
}

bool RegionCollection::Add(Region* region) {
	if (Has(region->GetLabel())) return false;

	regions.insert(std::pair<unsigned short, Region*>(region->GetLabel(), region));

	return true;
}

bool RegionCollection::Has(unsigned short label) {
	return regions.count(label) == 1;
}

Region* RegionCollection::Get(unsigned short label) {
	return Has(label) ? regions[label] : nullptr;
}

void RegionCollection::Remove(unsigned short label) {
	if (!Has(label)) return;

	delete regions[label];
	regions.erase(label);
}

void RegionCollection::RemoveAll() {
	InitTraversal();
	for (Region* region = GetNext(); region != nullptr; region = GetNext()) {
		delete region;
	}

	regions.clear();
}

int RegionCollection::Size() {
	return (int)regions.size();
}

void RegionCollection::InitTraversal() {
	it = regions.begin();
}

Region* RegionCollection::GetNext() {
	if (it == regions.end()) return nullptr;
	
	Region* region = it->second;
	it++;

	return region;
}

