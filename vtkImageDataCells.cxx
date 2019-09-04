#include "vtkImageDataCells.h"

#include "vtkExecutive.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
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
	return 1;
}

//----------------------------------------------------------------------------
int vtkImageDataCells::FillInputPortInformation(int vtkNotUsed(port), vtkInformation *info)
{
	info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
	return 1;
}
