#ifndef Probe_H
#define Probe_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkExtractVOI;
class vtkImageData;
class vtkImageDataCells;

class Probe {
public:
	Probe();
	~Probe();

	void UpdateData(vtkImageData* data);

	vtkActor* GetActor();
	
protected:
	vtkSmartPointer<vtkActor> actor;
};

#endif
