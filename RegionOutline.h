#ifndef RegionOutline_H
#define RegionOutline_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCutter;
class vtkPlane;

class Region;

class RegionOutline {
public:
	RegionOutline(Region* inputRegion, double color[3]);
	~RegionOutline();

	vtkSmartPointer<vtkActor> GetActor();

	void SetPlane(vtkPlane* plane);

protected:
	Region* region;

	vtkSmartPointer<vtkCutter> cut;
	vtkSmartPointer<vtkActor> actor;
};

#endif
