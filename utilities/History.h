#ifndef History_H
#define History_H

#include <vtkSmartPointer.h>

#include <deque>

class vtkImageData;

class History {
public:
	History(int maxLength);
	~History();

	void Push(vtkImageData* labels);
	void Undo(vtkImageData* labels);
	void Redo(vtkImageData* labels);
	void Clear();
	
protected:
	std::deque<vtkSmartPointer<vtkImageData>> history;

	int maxLength;
	int index;
};

#endif
