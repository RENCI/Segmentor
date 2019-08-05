#include "Region.h"

#include "vtkExtractVOI.h"
#include "vtkImageData.h"

#include <algorithm>

Region::Region(vtkImageData* data, unsigned short regionLabel) {
	// Input data info
	unsigned short* scalars = static_cast<unsigned short*>(data->GetScalarPointer());
	int numPoints = data->GetNumberOfPoints();

	int dataExtent[6];
	data->GetExtent(dataExtent);

	// Label for this region
	label = regionLabel;

	// Initialize extent for this region
	extent[0] = dataExtent[1];
	extent[1] = dataExtent[0];
	extent[2] = dataExtent[3];
	extent[3] = dataExtent[2];
	extent[4] = dataExtent[5];
	extent[5] = dataExtent[4];

	numVoxels = 0;
	for (int i = 0; i < numPoints; i++) {
		if (scalars[i] == label) {
			double* point = data->GetPoint(i);

			if (point[0] < extent[0]) extent[0] = point[0];
			if (point[0] > extent[1]) extent[1] = point[0];
			if (point[1] < extent[2]) extent[2] = point[1];
			if (point[1] > extent[3]) extent[3] = point[1];
			if (point[2] < extent[4]) extent[4] = point[2];
			if (point[2] > extent[5]) extent[5] = point[2];

			numVoxels++;
		}
	}

	// Add a buffer around VOI
	int padding = 2;
	extent[0] = std::max(dataExtent[0], extent[0] - padding);
	extent[1] = std::min(dataExtent[1], extent[1] + padding);
	extent[2] = std::max(dataExtent[2], extent[2] - padding);
	extent[3] = std::min(dataExtent[3], extent[3] + padding);
	extent[4] = std::max(dataExtent[4], extent[4] - padding);
	extent[5] = std::min(dataExtent[5], extent[5] + padding);

	voi = vtkSmartPointer<vtkExtractVOI>::New();
	voi->SetVOI(extent);
	voi->SetInputDataObject(data);
}
	
Region::~Region() {
}

vtkAlgorithmOutput* Region::GetOutput() {
	return voi->GetOutputPort();
}

unsigned short Region::GetLabel() {
	return label;
}

void Region::SetLabel(unsigned short regionLabel) {
	label = regionLabel;
}

int Region::GetNumVoxels() {
	return numVoxels;
}