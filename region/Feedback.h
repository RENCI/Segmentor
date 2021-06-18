#ifndef Feedback_H
#define Feedback_H

class Feedback {
public:
	Feedback();
	~Feedback();

	enum FeedbackType {
		Undertraced = 0,
		Overtraced,
		AddToSlice,
		RemoveId,
		Split,
		Merge,
		CorrectSplitMerge
	};

	void SetValue(FeedbackType type, bool value);
	bool GetValue(FeedbackType type);

	bool HasFeedback();

	void Copy(const Feedback& other);

protected:
	bool undertraced;
	bool overtraced;
	bool addToSlice;
	bool removeId;
	bool split;
	bool merge;
	bool correctSplitMerge;
};

#endif
