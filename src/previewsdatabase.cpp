#include "previewsdatabase.h"
#include "previewentry.h"
#include "validation.h"

#include "sqlite3.h"

namespace enlighten
{
namespace lib
{
PreviewsDatabase::PreviewsDatabase() : _sqliteDatabase(NULL)
{
}

PreviewsDatabase::~PreviewsDatabase()
{
	if (_sqliteDatabase)
	{
		sqlite3_close(_sqliteDatabase);
	}

	std::map<unsigned int, PreviewEntry*>::iterator it;
	for (it = _cachedEntries.begin(); it != _cachedEntries.end(); it++)
		delete it->second;
}

bool PreviewsDatabase::initialiseWithFile(const char* fileName)
{
	VALIDATE(fileName, "fileName argument is invalid");

	int dbOpenResult = sqlite3_open_v2(fileName, &_sqliteDatabase, SQLITE_OPEN_READONLY, NULL);
	VALIDATE(dbOpenResult == SQLITE_OK, "Failed to open sqlite database '%s'. Reason: %s",
		fileName, sqlite3_errmsg(_sqliteDatabase));

	return true;
}

sqlite3_stmt* PreviewsDatabase::makeStatement(const char* query)
{
	sqlite3_stmt* statement = nullptr;

	int statementResult = sqlite3_prepare_v2(_sqliteDatabase, query, -1, &statement,
		NULL);

	VALIDATE_AND_RETURN(nullptr, statementResult == SQLITE_OK, "Statement '%s' error. Reason: %s",
		query, sqlite3_errmsg(_sqliteDatabase));

	return statement;
}

unsigned int PreviewsDatabase::numberOfPreviewEntries()
{
	int numberOfRows = 0;

	VALIDATE_AND_RETURN(0, _sqliteDatabase, "Sqlite database is in an invalid state.");

	sqlite3_stmt* statement = makeStatement("SELECT COUNT(*) FROM ImageCacheEntry");
	if (!statement)
		return 0;

	if (sqlite3_step(statement) == SQLITE_ROW)
		numberOfRows = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	return numberOfRows;
}

const PreviewEntry* PreviewsDatabase::entryForIndex(unsigned int index)
{
	VALIDATE_AND_RETURN(NULL, _sqliteDatabase, "Sqlite database is in an invalid state.");

	std::string uuid, digest;
	std::vector<PreviewEntryLevel> levels;

	if (!selectImageCacheEntryColumnsForIndex(index, uuid, digest) ||
		!selectPyramidColumnsForUuid(uuid, levels))
	{
		return nullptr;
	}

	PreviewEntry* entry = new PreviewEntry(uuid, digest, levels);
	_cachedEntries.insert(std::make_pair(index, entry));

	return entry;
}

bool PreviewsDatabase::selectImageCacheEntryColumnsForIndex(unsigned int index,
	std::string& uuid, std::string& digest)
{
	const char* queryFormat = "SELECT uuid,digest FROM ImageCacheEntry LIMIT 1 OFFSET %d";

	char queryBuffer[128];
	int written = snprintf(queryBuffer, 128, queryFormat, index);
	VALIDATE(written < 128, "Buffer overflow");

	sqlite3_stmt* statement = makeStatement(queryBuffer);
	if (!statement)
		return false;

	bool recordsFound = false;
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		uuid   = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
		digest = reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));
		recordsFound = true;
	}

	sqlite3_finalize(statement);

	return recordsFound;
}

bool PreviewsDatabase::selectPyramidColumnsForUuid(const std::string& uuid,
	std::vector<PreviewEntryLevel>& levels)
{
	const char* queryFormat = "SELECT level,longDimension FROM PyramidLevel WHERE "
                               "uuid='%s'";

	char queryBuffer[128];
	int written = snprintf(queryBuffer, 128, queryFormat, uuid.c_str());
	VALIDATE(written < 128, "Buffer overflow");

	sqlite3_stmt* statement = makeStatement(queryBuffer);
	if (!statement)
		return false;

	bool recordsFound = false;
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		PreviewEntryLevel level(static_cast<int>(sqlite3_column_double(statement, 0)),
			sqlite3_column_double(statement, 1));

		levels.push_back(level);
		recordsFound = true;
	}

	sqlite3_finalize(statement);

	return recordsFound;
}
} // lib
} // enlighten
