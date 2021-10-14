#include "RegionInfo.h"

#include "Region.h"

RegionInfo::RegionInfo() : label(-1), visible(true), modified(false), done(false), verified(false), comment("") {
	color[0] = color[1] = color[2] = -1.0;
	extent[0] = extent[1] = extent[2] = extent[3] = extent[4] = extent[5] = -1;
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
	verified = region->verified;
	comment = region->comment;
}
	
RegionInfo::~RegionInfo() {
}