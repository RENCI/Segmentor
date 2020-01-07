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
	int numPoints = (inputDims[0] + 1) * (inputDims[1] + 1) * (inputDims[2] + 1);

	// Allocate memory for points
	vtkNew<vtkPoints> points;
	points->SetNumberOfPoints(numPoints);

	// Axes lengths for indexing
	int ni = inputDims[0];
	int nj = inputDims[1];

	// Point offset
	double halfCellX = spacing[0] / 2;
	double halfCellY = spacing[1] / 2;
	double halfCellZ = spacing[2] / 2;

	// Create points
	int n = 0;
	for (int k = 0; k <= inputDims[2]; k++) {
		for (int j = 0; j <= inputDims[1]; j++) {
			for (int i = 0; i <= inputDims[0]; i++) {
				// Defaults
				int i2 = i;
				int j2 = j;
				int k2 = k;

				double xOffset = -halfCellX;
				double yOffset = -halfCellY;
				double zOffset = -halfCellZ;

				// Edge cases
				if (i == inputDims[0]) {
					i2 = inputDims[0] - 1;
					xOffset = halfCellX;
				}

				if (j == inputDims[1]) {
					j2 = inputDims[1] - 1;
					yOffset = halfCellY;
				}

				if (k == inputDims[2]) {
					k2 = inputDims[2] - 1;
					zOffset = halfCellZ;
				}
				
				// Get point
				double p0[3];
				input->GetPoint(GetPointId(i2, j2, k2, ni, nj), p0);

				// Create offset point
				double p[3];				
				PointOffset(p0, xOffset, yOffset, zOffset, p);
				points->SetPoint(n++, p);
			}
		}
	}

	// Number of cells
	int numCells = input->GetNumberOfPoints();

	// Allocate memory for cells
	output->Allocate(numCells);

	// Axes lengths for indexing
	ni = inputDims[0] + 1;
	nj = inputDims[1] + 1;

	// Create cells
	for (int k = 0; k < inputDims[2]; k++) {
		for (int j = 0; j < inputDims[1]; j++) {
			for (int i = 0; i < inputDims[0]; i++) {
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
	output->GetCellData()->DeepCopy(input->GetPointData());

	// Set output points
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
