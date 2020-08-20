#ifndef Probe_H
#define Probe_H

#include <vtkSmartPointer.h>

class vtkActor;
class vtkExtractVOI;
class vtkImageData;
class vtkImageDataCells;

class Probe {
public:
	Probe(double probeScale = 1.0, bool probe3D = false);
	~Probe();

	void UpdateData(vtkImageData* data);

	vtkActor* GetActor();
	
protected:
	vtkSmartPointer<vtkActor> actor;

	double scale;
};

#endif
