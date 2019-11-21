#ifndef Region_H
#define Region_H

#include <vtkSmartPointer.h>

class vtkAlgorithmOutput;
class vtkExtractVOI;
class vtkImageData;
class vtkPlane;
class vtkThreshold;

class vtkImageDataCells;

class RegionSurface;
class RegionOutline;
class RegionVoxelOutlines;

class Region {
public:
	Region(unsigned short regionLabel, double regionColor[3], vtkImageData* data);
	~Region();

	vtkAlgorithmOutput* GetOutput();
	vtkAlgorithmOutput* GetCells();

	RegionSurface* GetSurface();
	RegionOutline* GetOutline();
	RegionVoxelOutlines* GetVoxelOutlines();

	void SetExtent(int newExtent[6]);
	void UpdateExtent(int x, int y, int z);

	void SetModified(bool isModified);
	void SetDone(bool isDone);

	unsigned short GetLabel();
	const double* GetColor();
	int GetNumVoxels();
	const int* GetExtent();

	bool GetModified();
	bool GetDone();

protected:
	unsigned short label;
	double color[3];
	int extent[6];

	bool modified;
	bool done;

	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkExtractVOI> voi;
	vtkSmartPointer<vtkThreshold> threshold;

	RegionSurface* surface;
	RegionOutline* outline;
	RegionVoxelOutlines* voxelOutlines;

	void UpdateExtent();
	void ClearLabels();
};

#endif
