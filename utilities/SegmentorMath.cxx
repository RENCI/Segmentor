#include "SegmentorMath.h"

#include <vtkImageData.h>

SegmentorMath::SegmentorMath() {
}

SegmentorMath::~SegmentorMath() {
}

SegmentorMath::OtsuValues SegmentorMath::OtsuThreshold(vtkImageData* image) {	
	// Based on code here: http://www.labbookpages.co.uk/software/imgProc/otsuThreshold.html	

	// Get image dimensions and extent
	int dims[3];
	image->GetDimensions(dims);

	int extent[6];
	image->GetExtent(extent);

	// Initialize values
	double minValue = image->GetScalarRange()[0];
	double maxValue = image->GetScalarRange()[1];

	if (minValue == maxValue) {
		OtsuValues otsuValues;
		otsuValues.threshold = minValue;
		otsuValues.backgroundMean = minValue;
		otsuValues.foregroundMean = minValue;

		return otsuValues;
	}

	const int count = dims[0] * dims[1] * dims[2];

	// Get values normalized to histogram bins
	std::vector<int> histogram(256, 0);
	std::vector<int> values(count, 0);

	int index = 0;
	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				double value = image->GetScalarComponentAsDouble(i, j, k, 0);
				values[index++] = (value - minValue) / (maxValue - minValue) * (histogram.size() - 1);
			}
		}
	}

	// Compute histogram
	for (int i = 0; i < values.size(); i++) {
		int v = values[i];
		histogram[v]++;
	}

	// Initialize values
	double sum = 0.0;
	for (int i = 0; i < histogram.size(); i++) {
		sum += i * histogram[i];
	}

	double sumB = 0.0;
	int wB = 0;
	int wF = 0;

	double varMax = 0.0;
	double threshold = 0.0;
	double meanB = 0.0;
	double meanF = 0.0;

	for (int t = 0; t < histogram.size(); t++) {
		// Weight Background
		wB += histogram[t];
		if (wB == 0) continue;

		// Weight Foreground
		wF = count - wB;
		if (wF == 0) break;

		sumB += (double)(t * histogram[t]);

		// Mean Background
		double mB = sumB / wB;

		// Mean Foreground
		double mF = (sum - sumB) / wF;

		// Calculate Between Class Variance
		double varBetween = (double)wB * (double)wF * (mB - mF) * (mB - mF);

		// Check if new maximum found
		if (varBetween > varMax) {
			varMax = varBetween;
			threshold = t;
			meanB = mB;
			meanF = mF;
		}
	}

	threshold = minValue + (maxValue - minValue) * threshold / histogram.size();
	meanB = minValue + (maxValue - minValue) * meanB / histogram.size();
	meanF = minValue + (maxValue - minValue) * meanF / histogram.size();

	//std::cout << "Threshold: " << threshold << std::endl;
	//std::cout << "Mean Background: " << meanB << std::endl;
	//std::cout << "Mean Foreground: " << meanF << std::endl;

	OtsuValues otsuValues;
	otsuValues.threshold = threshold;
	otsuValues.backgroundMean = meanB;
	otsuValues.foregroundMean = meanF;

	return otsuValues;
}

int SegmentorMath::Distance2(const Voxel& v1, const Voxel& v2) {
	int dx = v1.x - v2.x;
	int dy = v1.y - v2.y;
	int dz = v1.z - v2.z;

	return dx * dx + dy * dy + dz * dz;
}

// Based on code from:
// https://stackoverflow.com/questions/12367071/how-do-i-initialize-the-t-variables-in-a-fast-voxel-traversal-algorithm-for-ray
//
double SegmentorMath::MinBetween(vtkImageData* data, const Voxel& v1, const Voxel& v2) {
	#define SIGN(x) (x > 0 ? 1 : (x < 0 ? -1 : 0))
	#define FRAC0(x) (x - floorf(x))
	#define FRAC1(x) (1 - x + floorf(x))

	float tMaxX, tMaxY, tMaxZ, tDeltaX, tDeltaY, tDeltaZ;
	Voxel voxel;

	float x1 = v1.x;
	float y1 = v1.x;
	float z1 = v1.z;

	float x2 = v2.x;
	float y2 = v2.y;
	float z2 = v2.z;   

	int dx = SIGN(x2 - x1);
	if (dx != 0) tDeltaX = fmin(dx / (x2 - x1), 10000000.0f); else tDeltaX = 10000000.0f;
	if (dx > 0) tMaxX = tDeltaX * FRAC1(x1); else tMaxX = tDeltaX * FRAC0(x1);
	voxel.x = (int)x1;

	int dy = SIGN(y2 - y1);
	if (dy != 0) tDeltaY = fmin(dy / (y2 - y1), 10000000.0f); else tDeltaY = 10000000.0f;
	if (dy > 0) tMaxY = tDeltaY * FRAC1(y1); else tMaxY = tDeltaY * FRAC0(y1);
	voxel.y = (int)y1;

	int dz = SIGN(z2 - z1);
	if (dz != 0) tDeltaZ = fmin(dz / (z2 - z1), 10000000.0f); else tDeltaZ = 10000000.0f;
	if (dz > 0) tMaxZ = tDeltaZ * FRAC1(z1); else tMaxZ = tDeltaZ * FRAC0(z1);
	voxel.z = (int)z1;

	double min = VTK_DOUBLE_MAX;

	while (true) {
		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				voxel.x += dx;
				tMaxX += tDeltaX;
			}
			else {
				voxel.z += dz;
				tMaxZ += tDeltaZ;
			}
		}
		else {
			if (tMaxY < tMaxZ) {
				voxel.y += dy;
				tMaxY += tDeltaY;
			}
			else {
				voxel.z += dz;
				tMaxZ += tDeltaZ;
			}
		}
		if (tMaxX > 1 && tMaxY > 1 && tMaxZ > 1) break;
		// process voxel here
	}

	return min;
}