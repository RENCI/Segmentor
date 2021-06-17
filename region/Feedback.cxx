#include "Feedback.h"

#include <stdio.h>

Feedback::Feedback() {
	undertraced = false;
	overtraced = false;
	addToSlice = false;
	removeId = false;
	correctSplitMerge = false;
}

Feedback::~Feedback() {
}

void Feedback::SetValue(FeedbackType type, bool value) {
	switch (type) {
	case Undertraced: undertraced = value; break;
	case Overtraced: overtraced = value; break;
	case AddToSlice: addToSlice = value; break;
	case RemoveId: removeId = value; break;
	case CorrectSplitMerge: correctSplitMerge = value; break;
	}
}

bool Feedback::GetValue(FeedbackType type) {
	switch (type) {
	case Undertraced: return undertraced;
	case Overtraced: return overtraced;
	case AddToSlice: return addToSlice;
	case RemoveId: return removeId;
	case CorrectSplitMerge: return correctSplitMerge;
	default: return false;
	}
}

void Feedback::Copy(const Feedback& other) {
	undertraced = other.undertraced;
	overtraced = other.overtraced;
	addToSlice = other.addToSlice;
	removeId = other.removeId;
	correctSplitMerge = other.correctSplitMerge;
}