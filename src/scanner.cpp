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

	std::string basePath = rootPath;
	if (basePath.find_last_of(File::pathSeperator()) == std::string::npos)
	{
		basePath += File::pathSeperator();
	}

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

			// Strip path
			size_t lastSeperatorIdx = fileName.find_last_of(File::pathSeperator());
			if (lastSeperatorIdx != std::string::npos)
			{
				fileName = fileName.substr(lastSeperatorIdx);
			}

			// Strip extension
			fileName = fileName.substr(0, extensionPtr - entryName);

			// Strip 'Previews' from lrdata
			size_t previewsIdentIdx = fileName.rfind("Previews"); // TODO - localise
			if (previewsIdentIdx != std::string::npos)
			{
				fileName = fileName.substr(0, previewsIdentIdx-1);
			}

			auto it = pairs.find(fileName);
			if (it == pairs.end())
			{
				auto insertion = pairs.insert(std::make_pair(fileName, LightroomFilePair()));
				it = insertion.first;
			}


			switch (fileType)
			{
				case LrCat:
					it->second.catalog = basePath + entryName;
					break;
				case Preview:
					it->second.previews = basePath + entryName + File::pathSeperator() + "previews.db"; // TODO -localise
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
		LightroomFilePair& p = filePair.second;
		_filePairs.insert(p);
	}

	return pairs.size() > 0;
}

const Scanner::LightroomFilePairs& Scanner::lightroomFilePairs() const
{
	return _filePairs;
}
} // lib
} // enlighten
