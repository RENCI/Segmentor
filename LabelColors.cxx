#include "LabelColors.h"

#include "vtkLookupTable.h"
/*
LabelColors::LabelColors() {
}

LabelColors::~LabelColors() {
}

vtkSmartPointer<vtkLookupTable> GetColors(vtkImageData* data) {
	int maxLabel = data->GetScalarRange()[1];

	vtkSmartPointer<vtkLookupTable> labelColors = vtkSmartPointer<vtkLookupTable>::New();
	labelColors->SetNumberOfTableValues(maxLabel + 1);
	labelColors->SetRange(0, maxLabel);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0);
	for (int i = 1; i <= maxLabel; i++) {
		double* c = colors[(i - 1) % numColors];
		labelColors->SetTableValue(i, c[0] / 255.0, c[1] / 255.0, c[2] / 255.0);
	}
	labelColors->Build();

	return labelColors;
}
*/