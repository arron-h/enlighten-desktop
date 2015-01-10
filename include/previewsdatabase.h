#ifndef PREVIEWS_DATABASE_H
#define PREVIEWS_DATABASE_H

#include <string>
#include <map>
#include <vector>
#include "previewentrylevel.h"

struct sqlite3;
struct sqlite3_stmt;

namespace enlighten
{
namespace lib
{
class PreviewEntry;
class PreviewsDatabase
{
public:
	PreviewsDatabase();
	~PreviewsDatabase();

	bool initialiseWithFile(const char* fileName);

	unsigned int numberOfPreviewEntries();
	const PreviewEntry* entryForIndex(unsigned int index);

private:
	sqlite3_stmt* makeStatement(const char* query);
	bool selectImageCacheEntryColumnsForIndex(unsigned int index,
		std::string& uuid, std::string& digest);
	bool selectPyramidColumnsForUuid(const std::string& uuid,
		std::vector<PreviewEntryLevel>& levels);

private:
	std::map<unsigned int, PreviewEntry*> _cachedEntries;
	sqlite3* _sqliteDatabase;
};
} // lib
} // enlighten

#endif // PREVIEWS_DATABASE_H
