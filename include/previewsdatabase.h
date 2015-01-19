#ifndef PREVIEWS_DATABASE_H
#define PREVIEWS_DATABASE_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include "previewentrylevel.h"
#include "previewentry.h"
#include "syncaction.h"

struct sqlite3;
struct sqlite3_stmt;

namespace enlighten
{
namespace lib
{
class ICachedPreviews;
class PreviewsDatabase
{
public:
	PreviewsDatabase();
	~PreviewsDatabase();

	bool initialiseWithFile(const std::string& fileName);
	bool reopen();

	unsigned int numberOfPreviewEntries();
	bool uuidForIndex(uint32_t index, uuid_t& uuid);
	const PreviewEntry* entryForUuid(const uuid_t& uuid);

	bool checkEntriesAgainstCachedPreviews(const ICachedPreviews& cachedPreviews,
		std::map<uuid_t, SyncAction>& uuidActions);

private:
	sqlite3_stmt* makeStatement(const char* query);

	bool selectUuidColumnForIndex(uint32_t index, uuid_t& uuid);

	bool selectImageCacheEntryColumnsForUuid(const uuid_t& uuid,
		std::string& digest);
	bool selectPyramidColumnsForUuid(const uuid_t& uuid,
		std::vector<PreviewEntryLevel>& levels);

private:
	int32_t _cachedNumberOfEntries;
	std::string _sourceFile;
	std::map<uuid_t, PreviewEntry*>  _cachedEntries;

	sqlite3* _sqliteDatabase;

};
} // lib
} // enlighten

#endif // PREVIEWS_DATABASE_H
