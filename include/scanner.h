#ifndef SCANNER_H
#define SCANNER_H

#include <set>
#include <string>

namespace enlighten
{
namespace lib
{
class Scanner
{
public:
	struct LightroomFilePair
	{
		std::string catalog;
		std::string previews;
	};

	typedef std::set<LightroomFilePair> LightroomFilePairs;

public:
	Scanner();

	bool scanForLightroomFilesAtPath(const char* rootPath);
	const LightroomFilePairs& lightroomFilePairs() const;

private:
	LightroomFilePairs _filePairs;
};
} // lib
} // enlighten

#endif
