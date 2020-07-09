#ifndef Brush_H
#define Brush_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkExtractVOI;
class vtkImageData;
class vtkImageDataCells;

class Brush {
public:
	Brush();
	~Brush();

	void UpdateData(vtkImageData* data);

	void SetRadius(int brushRadius);
	int GetRadius();

	vtkActor* GetActor();

protected:
	int radius;

	vtkSmartPointer<vtkActor> actor;

	vtkSmartPointer<vtkExtractVOI> voi;
	vtkSmartPointer<vtkImageDataCells> cells;

	void UpdateBrush();
};

#endif
