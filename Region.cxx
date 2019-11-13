#include "Region.h"

#include "vtkExtractVOI.h"
#include "vtkImageData.h"

#include <algorithm>

Region::Region(vtkImageData* inputData, unsigned short regionLabel, double regionColor[3]) {
	done = false;

	// Input data info
	data = inputData;
	unsigned short* scalars = static_cast<unsigned short*>(data->GetScalarPointer());
	int numPoints = data->GetNumberOfPoints();

	int dataExtent[6];
	data->GetExtent(dataExtent);

	// Label for this region
	label = regionLabel;

	// Color for this region
	color[0] = regionColor[0];
	color[1] = regionColor[1];
	color[2] = regionColor[2];

	// Initialize extent for this region
	extent[0] = dataExtent[1];
	extent[1] = dataExtent[0];
	extent[2] = dataExtent[3];
	extent[3] = dataExtent[2];
	extent[4] = dataExtent[5];
	extent[5] = dataExtent[4];

	// XXX: USE GENERATE REGION EXTENTS FROM vktImageConnectivityFilter AND PASS THIS IN
	//		ALSO REGION SIZE FROM FILTER

	for (int i = 0; i < numPoints; i++) {
		if (scalars[i] == label) {
			double* point = data->GetPoint(i);

			if (point[0] < extent[0]) extent[0] = point[0];
			if (point[0] > extent[1]) extent[1] = point[0];
			if (point[1] < extent[2]) extent[2] = point[1];
			if (point[1] > extent[3]) extent[3] = point[1];
			if (point[2] < extent[4]) extent[4] = point[2];
			if (point[2] > extent[5]) extent[5] = point[2];
		}
	}

	voi = vtkSmartPointer<vtkExtractVOI>::New();
	voi->SetInputDataObject(data);

	UpdateExtent();
}
	
Region::~Region() {
	ClearLabels();
}

vtkAlgorithmOutput* Region::GetOutput() {
	return voi->GetOutputPort();
}

void Region::SetExtent(int newExtent[6]) {
	for (int i = 0; i < 6; i++) {
		extent[i] = newExtent[i];
	}

	UpdateExtent();
}

void Region::UpdateExtent(int x, int y, int z) {
	bool update = false;

	if (x < extent[0]) {
		extent[0] = x;
		update = true;
	}
	else if (x > extent[1]) {
		extent[1] = x;
		update = true;
	}

	if (y < extent[2]) {
		extent[2] = y;
		update = true;
	}
	else if (y > extent[3]) {
		extent[3] = y;
		update = true;
	}

	if (z < extent[4]) {
		extent[4] = z;
		update = true;
	}
	else if (z > extent[5]) {
		extent[5] = z;
		update = true;
	}

	if (update) {
		UpdateExtent();
	}
}

void Region::UpdateExtent() {
	int dataExtent[6];
	data->GetExtent(dataExtent);

	// Add a buffer around VOI
	const int padding = 2;

	int padExtent[6];
	padExtent[0] = std::max(dataExtent[0], extent[0] - padding);
	padExtent[1] = std::min(dataExtent[1], extent[1] + padding);
	padExtent[2] = std::max(dataExtent[2], extent[2] - padding);
	padExtent[3] = std::min(dataExtent[3], extent[3] + padding);
	padExtent[4] = std::max(dataExtent[4], extent[4] - padding);
	padExtent[5] = std::min(dataExtent[5], extent[5] + padding);

	voi->SetVOI(padExtent);
}

void Region::SetDone(bool isDone) {
	done = isDone;
}

unsigned short Region::GetLabel() {
	return label;
}

const double* Region::GetColor() {
	return color;
}

int Region::GetNumVoxels() {
	voi->Update();

	vtkImageData* voiData = voi->GetOutput();
	unsigned short* scalars = static_cast<unsigned short*>(voiData->GetScalarPointer());
	int numPoints = voiData->GetNumberOfPoints();

	int numVoxels = 0;
	for (int i = 0; i < numPoints; i++) {
		if (scalars[i] == label) numVoxels++;
	}

	return numVoxels;
}

const int* Region::GetExtent() {
	return extent;
}

bool Region::GetDone() {
	return done;
}

void Region::ClearLabels() {
	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, k));
				if (*p == label) *p = 0;
			}
		}
	}

	data->Modified();
}