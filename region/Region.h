#ifndef Region_H
#define Region_H

//#define SHOW_REGION_BOX

#include <vtkSmartPointer.h>

#include "RegionMetadataIO.h"

class vtkActor;
class vtkAlgorithmOutput;
class vtkExtractVOI;
class vtkImageData;
class vtkPlane;
class vtkTable;
class vtkBillboardTextActor3D;
class vtkThreshold;

class vtkImageDataCells;

class RegionInfo;
class RegionSurface;
class RegionOutline;
class RegionVoxelOutlines;
class RegionHighlight3D;

class Region {
public:
	Region(unsigned short regionLabel, double regionColor[3], vtkImageData* data, const int* regionExtent = nullptr);
	Region(const RegionInfo& info, vtkImageData* data);
	~Region();

	vtkAlgorithmOutput* GetOutput();
	vtkAlgorithmOutput* GetCells();

	vtkSmartPointer<vtkTable> GetPointTable();

	RegionSurface* GetSurface();
	RegionOutline* GetOutline();
	RegionVoxelOutlines* GetVoxelOutlines();
	RegionHighlight3D* GetHighlight3D();
	vtkSmartPointer<vtkBillboardTextActor3D> GetText();
	vtkSmartPointer<vtkImageData> GetZSlice(int z);

#ifdef SHOW_REGION_BOX
	vtkSmartPointer<vtkActor> GetBox();
#endif

	void InitializeExtent(const int* regionExtent);
	void SetExtent(int newExtent[6]);
	void UpdateExtent(int x, int y, int z);
	void ShrinkExtent();

	bool GetVisible();
	void SetVisible(bool isVisible);

	bool GetModified();
	void SetModified(bool isModified);
	
	bool GetDone();
	void SetDone(bool isDone);

	void SetColor(double r, double g, double b);
	const double* GetColor();

	void ShowText(bool show);

	unsigned short GetLabel();
	int GetNumVoxels();
	int GetNumVoxels(int slice);
	const int* GetExtent();
	void GetExtent(int outExtent[6]);
	double* GetCenter();
	double GetLength();

	double GetXYDistance(int x, int y, int z);

	bool GetSeed(double point[3]);
	bool GetSeed(double point[3], int z);

	void SetInfo(const RegionInfo& info);

protected:
	unsigned short label;
	double color[3];
	int extent[6];

	bool visible;
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

#ifdef SHOW_REGION_BOX
	vtkSmartPointer<vtkActor> box;
#endif

	void ComputeExtent();
	void ShrinkExtent(const int startExtent[6]);
	void UpdateExtent();

	void ClearLabels();

	friend class RegionInfo;
};

#endif
