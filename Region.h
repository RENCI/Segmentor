#ifndef Region_H
#define Region_H

#include "vtkSmartPointer.h"

class vtkAlgorithmOutput;
class vtkExtractVOI;
class vtkImageData;

class Region {
public:
	Region(vtkImageData* data, unsigned short regionLabel, double regionColor[3]);
	~Region();

	vtkAlgorithmOutput* GetOutput();

	void SetExtent(int newExtent[6]);
	void UpdateExtent(int x, int y, int z);

	unsigned short GetLabel();
	const double* GetColor();
	int GetNumVoxels();
	const int* GetExtent();

protected:
	unsigned short label;
	double color[3];
	int extent[6];

	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkExtractVOI> voi;

	void UpdateExtent();
};

#endif
