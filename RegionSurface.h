#ifndef RegionSurface_H
#define RegionSurface_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkLookupTable;

class Region;

class RegionSurface {
public:
	RegionSurface(Region* region, vtkLookupTable* lut);
	~RegionSurface();

	vtkSmartPointer<vtkActor> GetActor();

protected:
	vtkSmartPointer<vtkActor> actor;
};

#endif
