#ifndef RegionHighlight3D_H
#define RegionHighlight3D_H

#include "vtkSmartPointer.h"

class vtkActor;
class vtkCamera;
class vtkDiskSource;
class vtkFollower;

class Region;

class RegionHighlight3D {
public:
	RegionHighlight3D(Region* inputRegion, double color[3]);
	~RegionHighlight3D();

	void SetCamera(vtkCamera* camera);

	vtkSmartPointer<vtkActor> GetActor();

	void Update();

protected:
	Region* region;

	vtkSmartPointer<vtkDiskSource> disk;
	vtkSmartPointer<vtkFollower> actor;
};

#endif
