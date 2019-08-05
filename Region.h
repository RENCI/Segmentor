#ifndef Region_H
#define Region_H

#include "vtkSmartPointer.h"

class vtkAlgorithmOutput;
class vtkExtractVOI;
class vtkImageData;

class Region {
public:
	Region(vtkImageData* data, unsigned short regionLabel);
	~Region();

	vtkAlgorithmOutput* GetOutput();

	unsigned short GetLabel();
	void SetLabel(unsigned short regionLabel);

	int GetNumVoxels();

protected:
	unsigned short label;
	int extent[6];
	int numVoxels;

	vtkSmartPointer<vtkExtractVOI> voi;
};

#endif
