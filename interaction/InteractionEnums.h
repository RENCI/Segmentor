#ifndef InteractionEnums_H
#define InteractionEnums_H

enum InteractionMode {
	NavigationMode,
	EditMode,
	AddMode,
	UpdateMode,
	MergeMode,
	SplitMode,
	GrowMode,
	VisibleMode
};

enum FilterMode {
	FilterNone,
	FilterPlane,
	FilterNeighbors,
	FilterRegion
};

#endif