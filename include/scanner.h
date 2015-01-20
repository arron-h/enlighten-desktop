#ifndef SCANNER_H
#define SCANNER_H

namespace enlighten
{
namespace lib
{
class Scanner
{
public:
	enum FileType
	{
		LRCAT,
		LRDAT
	};

	typedef std::pair<std::string, std::string> LightroomFilePair;

public:
	Scanner();

	bool scanForLightroomFilesAtPath(const char* rootPath,
		std::vector<LightroomFilePair>& matchingFilePairs);

	void writePairsToSettings(const std::vector<LightroomFilePair>& pairs,
		EnlightenSettings& settings);
};
} // lib
} // enlighten

#endif
