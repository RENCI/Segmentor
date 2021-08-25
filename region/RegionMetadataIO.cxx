#include "RegionMetadataIO.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

#include <iostream>
#include <string>

#include "RegionInfo.h"

RegionMetadataIO::RegionMetadataIO() {
}

RegionMetadataIO::~RegionMetadataIO() {
}

std::vector<RegionInfo> RegionMetadataIO::Read(std::string fileName) {
	// Return vector
	std::vector<RegionInfo> regionData;

	QFile file(fileName.c_str());

	if (file.open(QIODevice::ReadOnly)) {
		QJsonDocument document(QJsonDocument::fromJson(file.readAll()));
		QJsonObject json = document.object();

		if (json.contains("regions") && json["regions"].isArray()) {
			QJsonArray regionObjects = json["regions"].toArray();

			for (int i = 0; i < regionObjects.size(); i++) {
				QJsonObject regionObject = regionObjects[i].toObject();

				RegionInfo region;

				if (regionObject.contains("label") && regionObject["label"].isDouble()) {
					region.label = (unsigned short)regionObject["label"].toDouble();
				}
				else {
					// Skip if no label
					continue;
				}

				if (regionObject.contains("visible") && regionObject["visible"].isBool()) {
					region.visible = regionObject["visible"].toBool();
				}

				if (regionObject.contains("modified") && regionObject["modified"].isBool()) {
					region.modified = regionObject["modified"].toBool();
				}

				if (regionObject.contains("done") && regionObject["done"].isBool()) {
					region.done = regionObject["done"].toBool();
				}

				if (regionObject.contains("color") && regionObject["color"].isArray()) {
					QJsonArray color = regionObject["color"].toArray();

					for (int i = 0; i < color.size() && i < 3; i++) {
						region.color[i] = color[i].toDouble();
					}
				}

				if (regionObject.contains("extent") && regionObject["extent"].isArray()) {
					QJsonArray extent = regionObject["extent"].toArray();

					for (int i = 0; i < extent.size() && i < 6; i++) {
						region.extent[i] = extent[i].toInt();
					}
				}

				if (regionObject.contains("comment") && regionObject["comment"].isString()) {
					region.comment = regionObject["comment"].toString().toStdString();
				}

				regionData.push_back(region);
			}
		}
	}
	else {
		std::cout << "No region metadata file found." << std::endl;
	}

	return regionData;
}

bool RegionMetadataIO::Write(std::string fileName, std::vector<RegionInfo> regions) {
	QFile file(fileName.c_str());

	if (!file.open(QIODevice::WriteOnly)) {
		std::cout << "Could not save region metadata file." << std::endl;
		return false;
	}

	QJsonArray regionObjects;
	for (int i = 0; i < (int)regions.size(); i++) {		
		QJsonObject regionObject;

		regionObject["label"] = regions[i].label;
		regionObject["visible"] = regions[i].visible;
		regionObject["modified"] = regions[i].modified;
		regionObject["done"] = regions[i].done;

		QJsonArray color;
		for (int j = 0; j < 3; j++) {
			color.append(regions[i].color[j]);
		}
		regionObject["color"] = color;

		QJsonArray extent;
		for (int j = 0; j < 6; j++) {
			extent.append(regions[i].extent[j]);
		}
		regionObject["extent"] = extent;

		regionObject["comment"] = QString::fromStdString(regions[i].comment);

		regionObjects.append(regionObject);
	}

	QJsonObject json;
	json["regions"] = regionObjects;

	QJsonDocument document(json);

	file.write(document.toJson());

	return true;
}