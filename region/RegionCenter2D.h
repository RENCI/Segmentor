#ifndef RegionCenter2D_H
#define RegionCenter2D_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCamera;
class vtkMapper;

class Region;

class RegionCenter2D {
public:
	RegionCenter2D(Region* inputRegion, double color[3]);
	~RegionCenter2D();

	vtkSmartPointer<vtkActor> GetActor();

	void Update(double z, double size);

protected:
	Region* region;

	vtkSmartPointer<vtkMapper> coneMapper;
	vtkSmartPointer<vtkMapper> sphereMapper;
	vtkSmartPointer<vtkActor> actor;
};

#endif
