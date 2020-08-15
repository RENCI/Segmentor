#include "RegionInfo.h"

#include "Region.h"

RegionInfo::RegionInfo() {
}

RegionInfo::RegionInfo(Region* region) {
	label = region->label;

	for (int i = 0; i < 3; i++) {
		color[i] = region->color[i];
	}

	for (int i = 0; i < 6; i++) {
		extent[i] = region->extent[i];
	}

	visible = region->visible;
	modified = region->modified;
	done = region->done;
}
	
RegionInfo::~RegionInfo() {
}