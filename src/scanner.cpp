#include "scanner.h"
#include "file.h"
#include <dirent.h>
#include <cstring>
#include <map>

namespace
{
	const char* lrCatExtension = ".lrcat";
	const char* lrPreviewsExtension = ".lrdat";
}

namespace enlighten
{
namespace lib
{
Scanner::Scanner()
{
}

bool Scanner::scanForLightroomFilesAtPath(const char* rootPath)
{
	struct dirent* directoryEntry;

	DIR* dir = opendir(rootPath);
	if (!dir)
		return false;

	std::map<std::string, LightroomFilePair> pairs;
	enum FileType
	{
		LrCat,
		Preview,

		Unknown
	};

	while ((directoryEntry = readdir(dir)), dir != NULL)
	{
		const char* entryName    = directoryEntry->d_name;
		const char* extensionPtr = nullptr;
		FileType fileType = Unknown;

		if ((extensionPtr = strstr(entryName, lrCatExtension)) != NULL)
		{
			fileType = LrCat;
		}
		else if ((extensionPtr = strstr(entryName, lrPreviewsExtension)) != NULL)
		{
			fileType = Preview;
		}

		if (fileType != Unknown)
		{
			// Get a displayable name
			std::string fileName = entryName;
			fileName = fileName.substr(fileName.find_last_of(File::pathSeperator()));
			fileName = fileName.substr(0, extensionPtr - entryName1);
		}
	}

	closedir(dir);

	return true;
}

const Scanner::LightroomFilePairs& Scanner::lightroomFilePairs() const
{
	return _filePairs;
}
} // lib
} // enlighten
