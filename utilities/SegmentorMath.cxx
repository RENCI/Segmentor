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

int SegmentorMath::Distance2(Voxel v1, Voxel v2) {
	int dx = v1.x - v2.x;
	int dy = v1.y - v2.y;
	int dz = v1.z - v2.z;

	return dx * dx + dy * dy + dz * dz;
}