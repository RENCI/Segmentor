#include "Feedback.h"

Feedback::Feedback() {
	underTraced = false;
	overTraced = false;
	addToSlice = false;
	removeId = false;
	correctSplitMerge = false;
}

Feedback::~Feedback() {
}

void Feedback::SetValue(FeedbackType type, bool value) {
	switch (type) {
	case Undertraced: underTraced = value; break;
	case Overtraced: overTraced = value; break;
	case AddToSlice: addToSlice = value; break;
	case RemoveId: removeId = value; break;
	case CorrectSplitMerge: correctSplitMerge = value; break;
	}
}

bool Feedback::GetValue(FeedbackType type) {
	switch (type) {
	case Undertraced: return underTraced;
	case Overtraced: return overTraced;
	case AddToSlice: return addToSlice;
	case RemoveId: return removeId;
	case CorrectSplitMerge: return correctSplitMerge;
	default: return false;
	}
}
