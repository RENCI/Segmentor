#ifndef LabelColors_H
#define LabelColors_H

#include <vtkSmartPointer.h>

class vtkLookupTable;

class LabelColors {
public:	
	static void Initialize();
	static double* GetColor(int index);

protected:
	static double colors[][3];

private:
	LabelColors();
	~LabelColors();
};


#endif
