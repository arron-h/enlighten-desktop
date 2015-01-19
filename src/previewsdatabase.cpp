#include "previewsdatabase.h"
#include "previewentry.h"
#include "validation.h"
#include "cachedpreviews.h"

#include "sqlite3.h"

namespace enlighten
{
namespace lib
{
PreviewsDatabase::PreviewsDatabase() : _sqliteDatabase(nullptr),
	_cachedNumberOfEntries(-1)
{
}

PreviewsDatabase::~PreviewsDatabase()
{
	if (_sqliteDatabase)
	{
		sqlite3_close(_sqliteDatabase);
	}

	for (auto it = _cachedEntries.begin(); it != _cachedEntries.end(); it++)
		delete it->second;
}

bool PreviewsDatabase::initialiseWithFile(const std::string& fileName)
{
	VALIDATE(fileName.length(), "Invalid filename");

	int dbOpenResult = sqlite3_open_v2(fileName.c_str(), &_sqliteDatabase,
		SQLITE_OPEN_READONLY, NULL);
	VALIDATE(dbOpenResult == SQLITE_OK, "Failed to open sqlite database '%s'. Reason: %s",
		fileName.c_str(), sqlite3_errmsg(_sqliteDatabase));

	_sourceFile = fileName;
	_cachedNumberOfEntries = -1;

	return true;
}

bool PreviewsDatabase::reopen()
{
	VALIDATE(_sourceFile.length() != 0 && _sqliteDatabase != nullptr, "Database is not already open");
	sqlite3_close(_sqliteDatabase);

	return initialiseWithFile(_sourceFile);
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
	if (_cachedNumberOfEntries >= 0)
		return static_cast<uint32_t>(_cachedNumberOfEntries);

	int numberOfRows = 0;

	VALIDATE_AND_RETURN(0, _sqliteDatabase, "Sqlite database is in an invalid state.");

	sqlite3_stmt* statement = makeStatement("SELECT COUNT(*) FROM ImageCacheEntry");
	if (!statement)
		return 0;

	if (sqlite3_step(statement) == SQLITE_ROW)
		numberOfRows = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	_cachedNumberOfEntries = numberOfRows;

	return numberOfRows;
}

bool PreviewsDatabase::uuidForIndex(uint32_t index, uuid_t& uuid)
{
	VALIDATE(_sqliteDatabase, "Sqlite database is in an invalid state.");
	VALIDATE(index < _cachedNumberOfEntries, "Sqlite database is in an invalid state.");

	return selectUuidColumnForIndex(index, uuid);
}

const PreviewEntry* PreviewsDatabase::entryForUuid(const uuid_t& uuid)
{
	VALIDATE_AND_RETURN(nullptr, _sqliteDatabase, "Sqlite database is in an invalid state.");

	auto it = _cachedEntries.find(uuid);
	if (it != _cachedEntries.end())
		return it->second;

	std::string digest;
	std::vector<PreviewEntryLevel> levels;

	if (!selectImageCacheEntryColumnsForUuid(uuid, digest) ||
		!selectPyramidColumnsForUuid(uuid, levels))
	{
		return nullptr;
	}

	PreviewEntry* entry = new PreviewEntry(uuid, digest, levels);
	_cachedEntries.insert(std::make_pair(uuid, entry));

	return entry;
}

bool PreviewsDatabase::checkEntriesAgainstCachedPreviews(const ICachedPreviews& cachedPreviews,
	std::map<uuid_t, SyncAction>& uuidActions)
{
	const char* query = "SELECT uuid FROM Pyramid";
	sqlite3_stmt* statement = makeStatement(query);
	CHECK(statement);

	// Because we need to remove or add a preview, we need to check in both directions
	// (db > cache and cache > db)
	std::set<uuid_t> cachedUuids;
	std::set<uuid_t>::iterator cachedIterator;
	cachedPreviews.generateProxy(cachedUuids);

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		const char* uuid = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
		cachedIterator = cachedUuids.find(uuid);
		if (cachedIterator == cachedUuids.end())
		{
			// Uuid not in cache - mark as add
			uuidActions.insert(std::make_pair(std::string(uuid), SyncAction_Add));
		}
		else
		{
			// Uuid in cache - do nothing
			cachedUuids.erase(cachedIterator);
		}
	}
	sqlite3_finalize(statement);

	// Now cachedUuids contains entries from the cache which no longer are in the previews.db
	for (auto uuid : cachedUuids)
	{
		uuidActions.insert(std::make_pair(uuid, SyncAction_Remove));
	}

	return true;
}

bool PreviewsDatabase::selectUuidColumnForIndex(uint32_t index, uuid_t& uuid)
{
	const char* queryFormat = "SELECT uuid FROM ImageCacheEntry LIMIT 1 OFFSET %d";

	char queryBuffer[128];
	int written = snprintf(queryBuffer, 128, queryFormat, index);
	VALIDATE(written < 128, "Buffer overflow");

	sqlite3_stmt* statement = makeStatement(queryBuffer);
	CHECK(statement);

	bool recordsFound = false;
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		uuid   = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
		recordsFound = true;
	}

	sqlite3_finalize(statement);

	return recordsFound;
}

bool PreviewsDatabase::selectImageCacheEntryColumnsForUuid(const uuid_t& uuid,
	std::string& digest)
{
	const char* queryFormat = "SELECT digest FROM ImageCacheEntry WHERE uuid='%s'";

	char queryBuffer[128];
	int written = snprintf(queryBuffer, 128, queryFormat, uuid.c_str());
	VALIDATE(written < 128, "Buffer overflow");

	sqlite3_stmt* statement = makeStatement(queryBuffer);
	CHECK(statement);

	bool recordsFound = false;
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		digest = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
		recordsFound = true;
	}

	sqlite3_finalize(statement);

	return recordsFound;
}

bool PreviewsDatabase::selectPyramidColumnsForUuid(const uuid_t& uuid,
	std::vector<PreviewEntryLevel>& levels)
{
	const char* queryFormat = "SELECT level,longDimension FROM PyramidLevel WHERE "
                               "uuid='%s'";

	char queryBuffer[128];
	int written = snprintf(queryBuffer, 128, queryFormat, uuid.c_str());
	VALIDATE(written < 128, "Buffer overflow");

	sqlite3_stmt* statement = makeStatement(queryBuffer);
	CHECK(statement);

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
