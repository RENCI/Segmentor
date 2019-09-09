#ifndef SliceLocation_H
#define SliceLocation_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkBox;
class vtkCamera;
class vtkCubeAxesActor;
class vtkCubeSource;
class vtkCutter;
class vtkFrustumSource;
class vtkImageData;
class vtkPlane;
class vtkPlaneSource;
class vtkPolyDataMapper;
class vtkRenderer;

class SliceLocation {
public:
	SliceLocation(vtkRenderer* ren);
	~SliceLocation();

	void UpdateData(vtkImageData* data);
	void UpdateView(vtkCamera* camera, vtkPlane* cutPlane);

protected:
	vtkSmartPointer<vtkRenderer> renderer;

	vtkSmartPointer<vtkCubeAxesActor> axes;

	vtkSmartPointer<vtkCutter> cutter;

	vtkSmartPointer<vtkPolyDataMapper> visibleMapper;
	vtkSmartPointer<vtkActor> visibleActor;

	vtkSmartPointer<vtkCubeSource> cubeSource;
	vtkSmartPointer<vtkPlaneSource> planeSource;
	vtkSmartPointer<vtkActor> plane;

	vtkSmartPointer<vtkActor> position;

	void CreateAxes();
	void CreatePlane();
	void CreatePosition();
	void CreateCameraActor();
};

#endif
