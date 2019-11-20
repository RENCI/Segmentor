#ifndef RegionVoxelOutlines_H
#define RegionVoxelOutlines_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCutter;
class vtkPlane;

class Region;

class RegionVoxelOutlines {
public:
	RegionVoxelOutlines(Region* inputRegion, double color[3]);
	~RegionVoxelOutlines();

	vtkSmartPointer<vtkActor> GetActor();

	void SetPlane(vtkPlane* plane);

protected:
	Region* region;

	vtkSmartPointer<vtkCutter> cut;
	vtkSmartPointer<vtkActor> actor;
};

#endif
