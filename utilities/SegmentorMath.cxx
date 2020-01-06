#include "SegmentorMath.h"

#include <vtkImageData.h>

SegmentorMath::SegmentorMath() {
}

SegmentorMath::~SegmentorMath() {
}


double SegmentorMath::OtsuThreshold(vtkImageData* image) {
	// Get image dimensions and extent
	int dims[3];
	image->GetDimensions(dims);

	int extent[6];
	image->GetExtent(extent);

	// Initialize values
	double minValue = image->GetScalarRange()[0];
	double maxValue = image->GetScalarRange()[1];
	double mean = 0.0;
	double sigma = 0.0;
	const int count = dims[0] * dims[1] * dims[2];

	// Bin values
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

	// Compute histogram and mean
	for (int i = 0; i < values.size(); i++) {
		int v = values[i];
		histogram[v]++;
		mean += v;
	}

	mean /= count;

	std::cout << "Mean: " << mean << std::endl;

	// Calculate sigma
	for (int i = 0; i < histogram.size(); i++) {
		double v = i - mean;

		sigma += v * v * histogram[i] / count;
	}

	std::cout << "Sigma: " << sigma << std::endl;

	// Calculate threshold bin
	int bin = 0;
	double criterion = 0.0;

	for (int i = 0; i < histogram.size(); i++) {
		double c = OtsuCriterion(image, histogram, values, sigma, count, i);
		if (c > criterion) {
			criterion = c;
			bin = i;
		}
	}

	std::cout << "Bin: " << bin << std::endl;

	double threshold = minValue + (maxValue - minValue) * bin / histogram.size();

	std::cout << "Threshold: " << threshold << std::endl;

	return threshold;
}

double SegmentorMath::OtsuCriterion(vtkImageData* image, const std::vector<int>& histogram, const std::vector<int>& values, double sigma, int count, int bin) {
	// Initialize values
	double meanA = 0.0;
	double wA = 0.0;
	int countA = 0;

	double meanB = 0.0;
	double wB = 0.0;
	int countB = 0;

	// Get class means
	for (int i = 0; i < values.size(); i++) {
		int v = values[i];
		if (v < bin) {
			meanA += v;
			countA++;
		}
		else {
			meanB += v;
			countB++;
		}
	}

	meanA /= countA;
	wA = (double)countA / count;

	meanB /= countB;
	wB = (double)countB / count;

	// Get the variance for each class
	double sigmaA = 0.0;
	double sigmaB = 0.0;

	for (int i = 0; i < bin; i++) {
		double v = i - meanA;

		sigmaA += v * v * histogram[i] / countA;
	}

	for (int i = bin; i < histogram.size(); i++) {
		double v = i - meanB;

		sigmaB += v * v * histogram[i] / countB;
	}

	// Get the inter-class variance
	double meanAB = meanA - meanB;
	double sigmaAB = wA * wB * meanAB * meanAB;
	double criterion = sigmaAB / sigma;

	return criterion;
}