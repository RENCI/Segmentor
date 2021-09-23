#ifndef RegionCenter3D_H
#define RegionCenter3D_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCamera;
class vtkSphereSource;

class Region;

class RegionCenter3D {
public:
	RegionCenter3D(Region* inputRegion, double color[3]);
	~RegionCenter3D();

	vtkSmartPointer<vtkActor> GetActor();

	void Update();

protected:
	Region* region;

	vtkSmartPointer<vtkSphereSource> sphere;
	vtkSmartPointer<vtkActor> actor;
};

#endif
