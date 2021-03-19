#ifndef RegionMetadataIO_H
#define RegionMetadataIO_H

#include <string>
#include <vector>

class RegionMetadataIO {
public:
	struct Region {
		Region() : label(-1), visible(true), modified(false), done(false) {
			extent[0] = extent[1] = extent[2] = extent[3] = extent[4] = extent[5] = 0;
		}

		unsigned short label;
		bool visible;
		bool modified;
		bool done;
		int extent[6];		
	};

	static std::vector<Region> Read(std::string fileName);
	static bool Write(std::string fileName, std::vector<Region> regions);

private:
	RegionMetadataIO();
	~RegionMetadataIO();
};

#endif
