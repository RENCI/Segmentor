#ifndef RegionMetadataIO_H
#define RegionMetadataIO_H

#include <string>
#include <vector>

#include <QJsonObject>

#include "Feedback.h"

class RegionInfo;

class RegionMetadataIO {
public:
	static std::vector<RegionInfo> Read(std::string fileName);
	static bool Write(std::string fileName, std::vector<RegionInfo> regions);

private:
	RegionMetadataIO();
	~RegionMetadataIO();

	static void SetFeedbackValue(RegionInfo& region, const QJsonObject& feedback, Feedback::FeedbackType type, const char* key);
};

#endif
