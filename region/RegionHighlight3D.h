#ifndef RegionHighlight3D_H
#define RegionHighlight3D_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCamera;
class vtkFollower;

class Region;

class RegionHighlight3D {
public:
	RegionHighlight3D(Region* inputRegion, double color[3], double width);
	~RegionHighlight3D();

	void SetCamera(vtkCamera* camera);

	vtkSmartPointer<vtkActor> GetActor();

protected:
	Region* region;

	vtkSmartPointer<vtkFollower> actor;
};

#endif
