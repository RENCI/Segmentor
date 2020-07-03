#include "History.h"

#include <algorithm>

#include <vtkImageData.h>

History::History(int maxLength) : maxLength(maxLength) {
	Clear();
}

History::~History() {
}

void History::Push(vtkImageData* labels) {
	vtkSmartPointer<vtkImageData> newLabels = vtkSmartPointer<vtkImageData>::New();

	newLabels->DeepCopy(labels);

	if (index < history.size() - 1) {
		history.erase(history.begin() + index + 1, history.end());
	}

	history.push_back(newLabels);

	if (history.size() > maxLength) history.pop_front();

	index = (int)history.size() - 1;

}

void History::Undo(vtkImageData* labels) {
	if (history.size() == 0) return;

	index = std::max(0, index - 1);

	labels->DeepCopy(history[index]);
}


void History::Redo(vtkImageData* labels) {
	if (history.size() == 0) return;

	index = std::min((int)history.size() - 1, index + 1);

	labels->DeepCopy(history[index]);
}

void History::Clear() {
	history.clear();
	index = -1;
}