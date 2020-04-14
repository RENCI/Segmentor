#ifndef SegmentorMath_H
#define SegmentorMath_H

#include <vector>

class vtkImageData;

class SegmentorMath {
public:
	struct OtsuValues {
		double threshold;
		double backgroundMean;
		double foregroundMean;
	};

	static OtsuValues OtsuThreshold(vtkImageData* image);

private:
	SegmentorMath();
	~SegmentorMath();
};

#endif