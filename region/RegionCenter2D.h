#ifndef RegionCenter2D_H
#define RegionCenter2D_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCamera;
class vtkSphereSource;

class Region;

class RegionCenter2D {
public:
	RegionCenter2D(Region* inputRegion, double color[3]);
	~RegionCenter2D();

	vtkSmartPointer<vtkActor> GetActor();

	void Update(double z);

protected:
	Region* region;

	vtkSmartPointer<vtkSphereSource> sphere;
	vtkSmartPointer<vtkActor> actor;
};

#endif
