#ifndef Region_H
#define Region_H

#include <vtkSmartPointer.h>

class vtkAlgorithmOutput;
class vtkExtractVOI;
class vtkImageData;
class vtkPlane;
class vtkThreshold;

class vtkImageDataCells;

class RegionOutline;
class RegionSurface;

class Region {
public:
	Region(unsigned short regionLabel, double regionColor[3], vtkImageData* data);
	~Region();

	vtkAlgorithmOutput* GetOutput();
	vtkAlgorithmOutput* GetCells();

	RegionOutline* GetOutline();
	RegionSurface* GetSurface();

	void SetExtent(int newExtent[6]);
	void UpdateExtent(int x, int y, int z);
	void SetDone(bool done);

	unsigned short GetLabel();
	const double* GetColor();
	int GetNumVoxels();
	const int* GetExtent();
	bool GetDone();

protected:
	unsigned short label;
	double color[3];
	int extent[6];
	bool done;

	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkExtractVOI> voi;
	vtkSmartPointer<vtkThreshold> threshold;

	RegionSurface* surface;
	RegionOutline* outline;

	void UpdateExtent();
	void ClearLabels();
};

#endif
