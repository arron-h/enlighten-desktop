#include "scanner.h"
#include "file.h"
#include "logger.h"
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

	while ((directoryEntry = readdir(dir)), dir != NULL && directoryEntry != NULL)
	{
		const char* entryName    = directoryEntry->d_name;
		const char* extensionPtr = nullptr;
		FileType fileType = Unknown;

		if (!entryName || entryName[0] == '\0')
			continue;

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
			size_t lastSeperatorIdx = fileName.find_last_of(File::pathSeperator());
			if (lastSeperatorIdx != std::string::npos)
			{
				fileName = fileName.substr(lastSeperatorIdx);
			}
			fileName = fileName.substr(0, extensionPtr - entryName);

			auto it = pairs.find(fileName);
			if (it == pairs.end())
			{
				auto insertion = pairs.insert(std::make_pair(fileName, LightroomFilePair()));
				it = insertion.first;
			}

			switch (fileType)
			{
				case LrCat:
					it->second.catalog = entryName;
					break;
				case Preview:
					it->second.previews = entryName;
					break;
				default:
					Logger::get().log(Logger::ERROR, "Invalid file type for file '%s'", entryName);
			}
		}
	}

	closedir(dir);

	// Copy to the set
	for (auto filePair : pairs)
	{
		LightroomFilePair p = filePair.second;

		LightroomFilePair pairlol;
		_filePairs.insert(pairlol);
	}

	return pairs.size() > 0;
}

const Scanner::LightroomFilePairs& Scanner::lightroomFilePairs() const
{
	return _filePairs;
}
} // lib
} // enlighten
