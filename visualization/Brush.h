#ifndef Brush_H
#define Brush_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkExtractVOI;
class vtkImageCast;
class vtkImageData;

class Brush {
public:
	Brush();
	~Brush();

	void UpdateData(vtkImageData* data);

	void UpdateBrush();

	void SetRadius(int brushRadius);
	int GetRadius();

	vtkActor* GetActor();

protected:
	int radius;

	vtkSmartPointer<vtkActor> actor;

	vtkSmartPointer<vtkImageCast> cast;
	vtkSmartPointer<vtkExtractVOI> voi;
};

#endif
