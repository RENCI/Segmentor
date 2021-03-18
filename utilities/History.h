#ifndef History_H
#define History_H

#include <vtkSmartPointer.h>

#include <deque>
#include <map>

class vtkImageData;

class RegionCollection;
class RegionInfo;

class History {
public:
	History(int maxLength);
	~History();

	void Push(vtkImageData* labels, RegionCollection* regions);
	void Head(vtkImageData* labels, RegionCollection* regions);
	void Undo(vtkImageData* labels, RegionCollection* regions);
	void Redo(vtkImageData* labels, RegionCollection* regions);
	void Clear();
	
protected:
	std::deque<vtkSmartPointer<vtkImageData>> labelHistory;

	typedef std::map<unsigned short, RegionInfo> RegionInfoCollection;
	std::deque<RegionInfoCollection> regionHistory;

	int maxLength;
	int index;

	void SaveInfo(RegionInfoCollection& info, RegionCollection* regions);
	void RestoreInfo(RegionCollection* regions, RegionInfoCollection& info, vtkImageData* labels);
};

#endif
