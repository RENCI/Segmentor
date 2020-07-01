#ifndef Region_H
#define Region_H

#include <vtkSmartPointer.h>

class vtkAlgorithmOutput;
class vtkExtractVOI;
class vtkImageData;
class vtkPlane;
class vtkTable;
class vtkBillboardTextActor3D;
class vtkThreshold;

class vtkImageDataCells;

class RegionSurface;
class RegionOutline;
class RegionVoxelOutlines;
class RegionHighlight3D;

class Region {
public:
	Region(unsigned short regionLabel, double regionColor[3], vtkImageData* data);
	~Region();

	vtkAlgorithmOutput* GetOutput();
	vtkAlgorithmOutput* GetCells();

	vtkSmartPointer<vtkTable> GetPointTable();

	RegionSurface* GetSurface();
	RegionOutline* GetOutline();
	RegionVoxelOutlines* GetVoxelOutlines();
	RegionHighlight3D* GetHighlight3D();
	vtkSmartPointer<vtkBillboardTextActor3D> GetText();

	void SetExtent(int newExtent[6]);
	void UpdateExtent(int x, int y, int z);
	void ComputeExtent();

	bool GetModified();
	void SetModified(bool isModified);
	
	bool GetDone();
	void SetDone(bool isDone);

	void ShowText(bool show);

	unsigned short GetLabel();
	const double* GetColor();
	int GetNumVoxels();
	const int* GetExtent();
	double* GetCenter();
	double GetLength();

	double GetXYDistance(int x, int y, int z);

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
	RegionHighlight3D* highlight3D;
	vtkSmartPointer<vtkBillboardTextActor3D> text;

	void UpdateExtent();
	void ClearLabels();
};

#endif
