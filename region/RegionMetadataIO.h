#ifndef RegionMetadataIO_H
#define RegionMetadataIO_H

#include <string>
#include <vector>

class RegionInfo;

class RegionMetadataIO {
public:
	static std::vector<RegionInfo> Read(std::string fileName);
	static bool Write(std::string fileName, std::vector<RegionInfo> regions);

private:
	RegionMetadataIO();
	~RegionMetadataIO();
};

#endif
