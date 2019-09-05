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
	/*
		// Copy input point and cell data to output
		output->GetPointData()->ShallowCopy(input->GetPointData());

		vtkIdType nbPoints = input->GetNumberOfPoints();

		// Extract points coordinates from the image
		vtkNew<vtkPoints> points;
		points->SetDataTypeToDouble();
		points->SetNumberOfPoints(nbPoints);
		for (vtkIdType i = 0; i < nbPoints; i++)
		{
			double p[3];
			input->GetPoint(i, p);
			points->SetPoint(i, p);
		}

		output->SetPoints(points.Get());

		output->Allocate(nbPoints);
		for (vtkIdType i = 0; i < nbPoints; i++)
		{
			vtkIdType p[1] = { i };
			output->InsertNextCell(VTK_VERTEX, 1, p);
		}
	*/

	// Input data information
	int* inputDims = input->GetDimensions();
	double* spacing = input->GetSpacing();



	std::cout << inputDims[0] << ", " << inputDims[1] << ", " << inputDims[2] << std::endl;
	std::cout << inputDims[0] * inputDims[1] * inputDims[2] << std::endl;
	std::cout << input->GetNumberOfPoints() << std::endl;
	std::cout << input->GetNumberOfCells() << std::endl;


	// Set number of output cells
	//int numCells = input->GetNumberOfPoints();
	int numCells = (inputDims[0] - 1) * (inputDims[1] - 1) * (inputDims[2] - 1);

	// Set number of output points	
	//int numPoints = (inputDims[0] + 1) * (inputDims[1] + 1) * (inputDims[2] + 1);
	int numPoints = input->GetNumberOfPoints();

	// Allocate memory for points
	vtkNew<vtkPoints> points;
	points->SetNumberOfPoints(numPoints);

	// Get data spacing
	double x = spacing[0] / 2;
	double y = spacing[1] / 2;
	double z = spacing[2] / 2;

	// Create points
	int n = 0;
	for (int k = 0; k < inputDims[2]; k++) {
		for (int j = 0; j < inputDims[1]; j++) {
			for (int i = 0; i < inputDims[0]; i++) {
				// Get input point
				int ijk[3] = { i, j, k };
				double p0[3];

				int id = (k * inputDims[1] + j) * inputDims[0] + i;
				input->GetPoint(id, p0);

				//std::cout << n << ", " << id << std::endl;



				//input->GetPoint(input->ComputePointId(ijk), p0);
				//input->GetPoint(n, p0);
				//std::cout << n << ", " << input->ComputePointId(ijk) << std::endl;

				// Create output point
				double p[3];
				PointOffset(p0, -x, -y, -z, p);
				points->SetPoint(n++, p);
			}
		}
	}

	// Allocate memory for cells
	//vtkNew<vtkCellArray> cells;
	//cells->Allocate(numCells * 8);
	output->Allocate(numCells * 8);
/*
	for (vtkIdType i = 0; i < numPoints; i++)
	{
		vtkIdType p[1] = { i };
		output->InsertNextCell(VTK_VERTEX, 1, p);
	}
*/
	for (int k = 0; k < inputDims[2] - 1; k++) {
		for (int j = 0; j < inputDims[1] - 1; j++) {
			for (int i = 0; i < inputDims[0] - 1; i++) {
				// Get point ids
				vtkIdType cellPoints[8];
				int ijk[8][3] = {
					{ i,	 j    ,	k     },
					{ i + 1, j    , k     },
					{ i + 1, j + 1, k     },
					{ i,     j + 1, k     },
					{ i,	 j    ,	k + 1 },
					{ i + 1, j    , k + 1 },
					{ i + 1, j + 1, k + 1 },
					{ i,     j + 1, k + 1 },
				};
				
				for (int m = 0; m < 8; m++) {
					int id = (k * inputDims[1] + j) * inputDims[0] + i;
					cellPoints[m] = id;

					if (id < 0 || id >= numPoints) std::cout << "PROBLEM: " << id << std::endl;
					//cellPoints[m] = input->ComputePointId(ijk[m]);
				}
				
				output->InsertNextCell(VTK_HEXAHEDRON, 8, cellPoints);
			}
		}
	}

	// Copy point data to cell data
	//output->GetCellData()->DeepCopy(input->GetPointData());
	//output->GetPointData()->ShallowCopy(input->GetPointData());
	//output->GetCellData()->CopyAllocate(input->GetPointData());

	// Set output
	output->SetPoints(points.Get());	
	//output->SetCells(VTK_HEXAHEDRON, cells.Get());

	output->Squeeze();
	
	return 1;
}

void vtkImageDataCells::PointOffset(double in[3], double x, double y, double z, double out[3]) {
	out[0] = in[0] + x;
	out[1] = in[1] + y;
	out[2] = in[2] + z;
}

//----------------------------------------------------------------------------
int vtkImageDataCells::FillInputPortInformation(int vtkNotUsed(port), vtkInformation *info)
{
	info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
	return 1;
}
