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
class vtkTextActor;
class vtkThreshold;

class vtkImageDataCells;

class RegionInfo;
class RegionSurface;
class RegionOutline;
class RegionVoxelOutlines;
class RegionHighlight3D;
class RegionCenter3D;
class RegionCenter2D;
class Feedback;

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
	RegionCenter3D* GetCenter3D();
	RegionCenter2D* GetCenter2D();
	vtkSmartPointer<vtkTextActor> GetText();
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

	bool GetVerified();
	void SetVerified(bool isVerified);

	void SetColor(double r, double g, double b);
	const double* GetColor();
	const double* GetDisplayedColor();

	void ShowText(bool show);

	void ShowCenter(bool show);

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

	bool HasComment();
	const std::string& GetComment();
	void SetComment(const std::string& commentString);

	void ApplyDot(double dotSize);

protected:
	unsigned short label;
	double color[3];
	int extent[6];
	double center[3];

	bool visible;
	bool modified;
	bool done;
	bool verified;
	std::string comment;

	vtkSmartPointer<vtkImageData> data;
	vtkSmartPointer<vtkExtractVOI> voi;
	vtkSmartPointer<vtkThreshold> threshold;

	RegionSurface* surface;
	RegionOutline* outline;
	RegionVoxelOutlines* voxelOutlines;
	RegionHighlight3D* highlight3D;
	RegionCenter3D* center3D;
	RegionCenter2D* center2D;
	vtkSmartPointer<vtkTextActor> text;

#ifdef SHOW_REGION_BOX
	vtkSmartPointer<vtkActor> box;
#endif

	void ComputeExtent();
	void ShrinkExtent(const int startExtent[6]);
	void UpdateExtent();

	void ClearLabels();

	void UpdateColor();

	void CreateText();

	std::string LabelString();

	friend class RegionInfo;
};

#endif
