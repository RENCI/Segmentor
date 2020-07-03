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

	Region* region = regions[label];

	delete region;

	regions.erase(label);
}

void RegionCollection::RemoveAll() {
	for (Iterator it = Begin(); it != End(); it++) {
		delete Get(it);
	}

	regions.clear();
}

int RegionCollection::Size() {
	return (int)regions.size();
}

RegionCollection::Iterator RegionCollection::Begin() {
	return regions.begin();
}

RegionCollection::Iterator RegionCollection::End() {
	return regions.end();
}

Region* RegionCollection::Get(RegionCollection::Iterator iterator) {
	return iterator->second;
}

unsigned short RegionCollection::GetNewLabel() {
	unsigned short label = 1;
	for (Iterator it = Begin(); it != End(); it++, label++) {
		if (it->first != label) return label;
	}

	return label;
}
