#ifndef RegionMetadataIO_H
#define RegionMetadataIO_H

#include <string>
#include <vector>

class RegionMetadataIO {
public:
	struct Region {
		Region() : label(-1), modified(false), done(false) {}
		unsigned short label;
		bool visible;
		bool modified;
		bool done;
	};

	static std::vector<Region> Read(std::string fileName);
	static bool Write(std::string fileName, std::vector<Region> regions);

private:
	RegionMetadataIO();
	~RegionMetadataIO();
};

#endif
