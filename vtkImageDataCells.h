#ifndef vtkImageDataCells_h
#define vtkImageDataCells_h

#include <vtkSetGet.h>
#include <vtkUnstructuredGridAlgorithm.h>

class vtkImageDataCells : public vtkUnstructuredGridAlgorithm
{
public:
	static vtkImageDataCells* New();
	vtkTypeMacro(vtkImageDataCells, vtkUnstructuredGridAlgorithm);

protected:
	vtkImageDataCells();
	~vtkImageDataCells() override ;

	// Usual data generation methods
	int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
	int FillInputPortInformation(int port, vtkInformation *info) override;

	void PointOffset(double in[3], double x, double y, double z, double out[3]);

	vtkIdType GetPointId(int i, int j, int k, int n1, int n2);

private:
	vtkImageDataCells(const vtkImageDataCells&) = delete;
	void operator=(const vtkImageDataCells&) = delete;
};

#endif
