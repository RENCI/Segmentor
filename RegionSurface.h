#ifndef RegionSurface_H
#define RegionSurface_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkDiscreteFlyingEdges3D;
class vtkLookupTable;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkWindowedSincPolyDataFilter;

class Region;

class RegionSurface {
public:
	RegionSurface(Region* inputRegion, vtkLookupTable* lut);
	~RegionSurface();

	Region* GetRegion();
	vtkSmartPointer<vtkActor> GetActor();

	void SetSmoothSurfaces(bool smooth);
	void SetSmoothShading(bool smooth);

protected:
	Region* region;

	bool smoothSurfaces;
	bool smoothShading;

	vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour;
	vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother;
	vtkSmartPointer<vtkPolyDataNormals> normals;
	vtkSmartPointer<vtkPolyDataMapper> mapper;
	vtkSmartPointer<vtkActor> actor;

	void UpdatePipeline();
};

#endif
