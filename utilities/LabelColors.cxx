#include "LabelColors.h"

#include "vtkLookupTable.h"

double LabelColors::doneColor[3] = { 1.0, 1.0, 1.0 };
double LabelColors::verifiedColor[3] = { 136 / 255.0, 139 / 255.0, 141 / 255.0 };

// Colors from ColorBrewer
const int numColors = 12;
double LabelColors::colors[numColors][3] = {
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

LabelColors::LabelColors() {
}

LabelColors::~LabelColors() {
}

void LabelColors::Initialize() {
	for (int i = 0; i < numColors; i++) {
		for (int j = 0; j < 3; j++) {
			colors[i][j] /= 255.0;
		}
	}
}

double* LabelColors::GetColor(int index) {
	return colors[index % numColors];
}