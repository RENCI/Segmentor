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

	struct Voxel {
		int x;
		int y;
		int z;
		unsigned short label;
	};

	static int Distance2(Voxel v1, Voxel v2);

private:
	SegmentorMath();
	~SegmentorMath();
};

#endif