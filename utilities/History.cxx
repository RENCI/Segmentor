#include "History.h"

#include <algorithm>
#include <utility>

#include <vtkImageData.h>

#include "Region.h"
#include "RegionCollection.h"
#include "RegionInfo.h"

History::History(int maxLength) : maxLength(maxLength) {
	Clear();
}

History::~History() {
}

void History::Push(vtkImageData* labels, RegionCollection* regions) {
	vtkSmartPointer<vtkImageData> newLabels = vtkSmartPointer<vtkImageData>::New();
	newLabels->DeepCopy(labels);

	RegionInfoCollection newInfo;
	SaveInfo(newInfo, regions);

	if (index < labelHistory.size() - 1) {
		labelHistory.erase(labelHistory.begin() + index + 1, labelHistory.end());
		regionHistory.erase(regionHistory.begin() + index + 1, regionHistory.end());
	}

	labelHistory.push_back(newLabels);
	regionHistory.push_back(newInfo);

	if (labelHistory.size() > maxLength) {
		labelHistory.pop_front();
		regionHistory.pop_front();
	}

	index = (int)labelHistory.size() - 1;
}

void History::Head(vtkImageData* labels, RegionCollection* regions) {
	if (labelHistory.size() == 0) return;

	labels->DeepCopy(labelHistory[index]);
	RestoreInfo(regions, regionHistory[index], labels);
}

void History::Undo(vtkImageData* labels, RegionCollection* regions) {
	if (labelHistory.size() == 0 || index == 0) return;

	index--;

	labels->DeepCopy(labelHistory[index]);
	RestoreInfo(regions, regionHistory[index], labels);
}


void History::Redo(vtkImageData* labels, RegionCollection* regions) {
	if (labelHistory.size() == 0 || index == labelHistory.size() - 1) return;

	index++;

	labels->DeepCopy(labelHistory[index]);
	RestoreInfo(regions, regionHistory[index], labels);
}

void History::Clear() {
	labelHistory.clear();
	regionHistory.clear();
	index = -1;
}

void History::SaveInfo(RegionInfoCollection& info, RegionCollection* regions) {
	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);
		info.insert(std::pair<unsigned short, RegionInfo>(region->GetLabel(), RegionInfo(region)));
	}
}

void History::RestoreInfo(RegionCollection* regions, RegionInfoCollection& info, vtkImageData* labels) {
	// Copy info for regions
	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End();) {
		Region* region = regions->Get(it++);

		unsigned short label = region->GetLabel();

		if (info.count(label) == 1) {
			region->SetInfo(info[label]);
		}
		else {
			regions->Remove(region->GetLabel());
		}
	}

	// Create new regions for any leftover
	for (RegionInfoCollection::iterator it = info.begin(); it != info.end(); it++) {
		if (!regions->Has(it->first)) {
			Region* region = new Region(it->second, labels);
			regions->Add(region);
		}
	}
}