#include "vtkImageDataCells.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkImageDataCells);

//----------------------------------------------------------------------------
vtkImageDataCells::vtkImageDataCells()
{
}

//----------------------------------------------------------------------------
vtkImageDataCells::~vtkImageDataCells()
{
}

//----------------------------------------------------------------------------
// Append data sets into single unstructured grid
int vtkImageDataCells::RequestData(
	vtkInformation *vtkNotUsed(request),
	vtkInformationVector **inputVector,
	vtkInformationVector *outputVector)
{
	vtkImageData* input = vtkImageData::GetData(inputVector[0], 0);
	vtkUnstructuredGrid* output = vtkUnstructuredGrid::GetData(outputVector, 0);

	if (input == nullptr)
	{
		vtkErrorMacro(<< "Input data is nullptr.");
		return 0;
	}
	if (input->GetDataDimension() != 3)
	{
		vtkErrorMacro(<< "Cannot convert non 3D image data!");
		return 0;
	}
	if (output == nullptr)
	{
		vtkErrorMacro(<< "Output data is nullptr.");
		return 0;
	}

	// Input data information
	int* inputDims = input->GetDimensions();
	double* spacing = input->GetSpacing();

	// Number of points
	int numPoints = input->GetNumberOfPoints();

	// Allocate memory for points
	vtkNew<vtkPoints> points;
	points->SetNumberOfPoints(numPoints);

	// Axes lengths for indexing
	int ni = inputDims[0];
	int nj = inputDims[1];

	// Point offset
	double x = spacing[0] / 2;
	double y = spacing[1] / 2;
	double z = spacing[2] / 2;

	// Create points
	for (int i = 0; i < inputDims[0]; i++) {
		for (int j = 0; j < inputDims[1]; j++) {
			for (int k = 0; k < inputDims[2]; k++) {
				vtkIdType id = GetPointId(i, j, k, ni, nj);

				double p0[3];
				input->GetPoint(id, p0);

				double p[3];
				PointOffset(p0, -x, -y, -z, p);

				points->SetPoint(id, p);
			}
		}
	}

	// Number of cells
	const int numCells = (inputDims[0] - 1) * (inputDims[1] - 1) * (inputDims[2] - 1);

	// Allocate memory for cells
	output->Allocate(numCells);

	// Create cells
	for (int i = 0; i < inputDims[0] - 1; i++) {
		for (int j = 0; j < inputDims[1] - 1; j++) {
			for (int k = 0; k < inputDims[2] - 1; k++) {
				vtkIdType cellPoints[8] = {
					GetPointId(i,     j,     k, ni, nj), 
					GetPointId(i + 1, j,     k, ni, nj), 
					GetPointId(i + 1, j + 1, k, ni, nj), 
					GetPointId(i,     j + 1, k, ni, nj),

					GetPointId(i,     j,     k + 1, ni, nj),
					GetPointId(i + 1, j,     k + 1, ni, nj),
					GetPointId(i + 1, j + 1, k + 1, ni, nj),
					GetPointId(i,     j + 1, k + 1, ni, nj),
				};

				output->InsertNextCell(VTK_HEXAHEDRON, 8, cellPoints);
			}
		}
	}

	// Copy point data to cell data 
	// XXX: Should be able to use shallow copy when drawing cells for each point
	vtkPointData* pointData = input->GetPointData();
	vtkCellData* cellData = output->GetCellData();

	cellData->CopyAllocate(pointData);
	
	int cid = 0;
	for (int i = 0; i < inputDims[0] - 1; i++) {
		for (int j = 0; j < inputDims[1] - 1; j++) {
			for (int k = 0; k < inputDims[2] - 1; k++) {
				vtkIdType pid = GetPointId(i, j, k, ni, nj);

				cellData->CopyData(pointData, pid, cid++);
			}
		}
	}

	// Set output
	//output->GetCellData()->DeepCopy(input->GetPointData());
	output->SetPoints(points.Get());

	return 1;
}

void vtkImageDataCells::PointOffset(double in[3], double x, double y, double z, double out[3]) {
	out[0] = in[0] + x;
	out[1] = in[1] + y;
	out[2] = in[2] + z;
}

vtkIdType vtkImageDataCells::GetPointId(int i, int j, int k, int ni, int nj) {
	return i + ni * (j + nj * k);
}

//----------------------------------------------------------------------------
int vtkImageDataCells::FillInputPortInformation(int vtkNotUsed(port), vtkInformation *info)
{
	info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
	return 1;
}
