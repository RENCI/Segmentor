#ifndef LabelColors_H
#define LabelColors_H

#include <vtkSmartPointer.h>

class vtkLookupTable;

class LabelColors {
public:	
	static vtkSmartPointer<vtkLookupTable> GetColors(vtkImageData* data);

	static void InitializeColors();

protected:
	// Colors from ColorBrewer
	static const int numColors = 12;
	static double colors[numColors][3] = {
		{ 166,206,227 },
		{ 31,120,180 },
		{ 178,223,138 },
		{ 51,160,44 },
		{ 251,154,153 },
		{ 227,26,28 },
		{ 253,191,111 },
		{ 255,127,0 },
		{ 202,178,214 },
		{ 106,61,154 },
		{ 255,255,153 },
		{ 177,89,40 }
	};

private:
	LabelColors();
	~LabelColors();
};

#endif
