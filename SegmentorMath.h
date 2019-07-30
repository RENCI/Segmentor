#ifndef SegmentorMath_H
#define SegmentorMath_H

#include <vector>

class vtkImageData;

class SegmentorMath {
public:
	static double OtsuThreshold(vtkImageData* image);	

private:
	SegmentorMath();
	~SegmentorMath();

	static double OtsuCriterion(vtkImageData* image, const std::vector<int>& histogram, const std::vector<int>& values, double sigma, int count, int bin);
};

#endif