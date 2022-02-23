#ifndef RegionSurface_H
#define RegionSurface_H

#include "vtkSmartPointer.h"

class vtkActor;
class myDiscreteFlyingEdges3D;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkWindowedSincPolyDataFilter;

class Region;

class RegionSurface {
public:
	RegionSurface(Region* inputRegion, double color[3]);
	~RegionSurface();

	vtkSmartPointer<vtkActor> GetActor();

	void SetSmoothSurface(bool smooth);
	void SetSmoothShading(bool smooth);

	enum RenderMode {
		Normal,
		Wireframe,
		CullFrontFace
	};

	void SetRenderMode(RenderMode mode);

	bool IntersectsPlane(double p[3], double n[3]);

protected:
	Region* region;

	bool smoothSurface;
	bool smoothShading;

	vtkSmartPointer<myDiscreteFlyingEdges3D> contour;
	vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother;
	vtkSmartPointer<vtkPolyDataNormals> normals;
	vtkSmartPointer<vtkPolyDataMapper> mapper;
	vtkSmartPointer<vtkActor> actor;

	void UpdatePipeline();

	bool IntersectsBoundingBox(double p[3], double n[3]);
	bool IntersectsSurface(double p[3], double n[3]);
};

#endif
