#ifndef SliceLocation_H
#define SliceLocation_H

#include <vtkSmartPointer.h>

class vtkBox;
class vtkCamera;
class vtkAxesActor;
class vtkCubeSource;
class vtkCutter;
class vtkImageData;
class vtkLineSource;
class vtkOutlineCornerFilter;
class vtkOutlineFilter;
class vtkPlane;
class vtkPlaneSource;
class vtkRenderer;

class SliceLocation {
public:
	SliceLocation(vtkRenderer* ren);
	~SliceLocation();

	void UpdateData(vtkImageData* data);
	void UpdateView(vtkCamera* camera, vtkPlane* cutPlane);

protected:
	double outlineColor[3];
	double sliceColor[3];

	double length;

	vtkSmartPointer<vtkRenderer> renderer;

	// Outline
	vtkSmartPointer<vtkOutlineFilter> outline;

	// Corners
	vtkSmartPointer<vtkOutlineCornerFilter> corners;

	// Plane
	vtkSmartPointer<vtkCubeSource> planeCube;
	vtkSmartPointer<vtkCutter> planeCutter;

	// Plane inset
	vtkSmartPointer<vtkPlaneSource> insetPlane;
	vtkSmartPointer<vtkBox> box;

	// View direction
	vtkSmartPointer<vtkLineSource> lineSource;

	// Axes
	vtkSmartPointer<vtkAxesActor> axes;

	void CreateOutline();
	void CreateCorners();
	void CreatePlane();
	void CreatePlaneInset();
	void CreateViewDirection();
	void CreateAxes();
};

#endif
