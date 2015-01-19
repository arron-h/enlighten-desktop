#include "cachedpreviews.h"
#include "settings.h"
#include "validation.h"

#include <cstdlib>
#include <chrono>

#include "sqlite3.h"

namespace enlighten
{
namespace lib
{
CachedPreviews::CachedPreviews(IEnlightenSettings* settings) : _sqliteDatabase(nullptr),
	_settings(settings), _lastChecked(0)
{
}

CachedPreviews::~CachedPreviews()
{
	if (_sqliteDatabase)
		sqlite3_close(_sqliteDatabase);
}

bool CachedPreviews::loadOrCreateDatabase()
{
	std::string filePath = _settings->get(EnlightenSettings::CachedDatabasePath, "");
	filePath += databaseFileName();

	int dbOpenResult = sqlite3_open_v2(filePath.c_str(), &_sqliteDatabase,
		SQLITE_OPEN_READWRITE, NULL);

	if (dbOpenResult != SQLITE_OK)
		return createCachedPreviewsDatabase(filePath);

	return true;
}

void CachedPreviews::previewsChecked()
{
	auto fromEpoch = std::chrono::system_clock::now().time_since_epoch();
	_lastChecked = std::chrono::duration_cast<std::chrono::seconds>(fromEpoch).count();
}

uint64_t CachedPreviews::lastCachedTime() const
{
	return _lastChecked;
}

uint32_t CachedPreviews::numberOfCachedPreviews() const
{
	int numberOfRows = 0;

	VALIDATE_AND_RETURN(0, _sqliteDatabase, "Sqlite database is in an invalid state.");

	sqlite3_stmt* statement = nullptr;
	const char* query = "SELECT COUNT(*) FROM PreviewsCache";
	int statementResult = sqlite3_prepare_v2(_sqliteDatabase, query, -1, &statement,
		NULL);

	VALIDATE_AND_RETURN(0, statementResult == SQLITE_OK, "Statement '%s' error. Reason: %s",
		query, sqlite3_errmsg(_sqliteDatabase));

	if (sqlite3_step(statement) == SQLITE_ROW)
		numberOfRows = sqlite3_column_int(statement, 0);

	sqlite3_finalize(statement);

	return numberOfRows;
}

bool CachedPreviews::isInCache(const uuid_t& uuid) const
{
	VALIDATE(_sqliteDatabase, "Sqlite database is in an invalid state.");

	const char* queryFormat = "SELECT uuid FROM PreviewsCache WHERE "
                               "uuid='%s'";

	char query[128];
	int written = snprintf(query, 128, queryFormat, uuid.c_str());
	VALIDATE(written < 128, "Buffer overflow");

	return executeAndCheckQuery(query, SQLITE_ROW);
}

bool CachedPreviews::markAsCached(const uuid_t& uuid)
{
	VALIDATE(_sqliteDatabase, "Sqlite database is in an invalid state.");

	const char* queryFormat = "INSERT INTO PreviewsCache (uuid) VALUES "
                               "('%s')";

	char query[128];
	int written = snprintf(query, 128, queryFormat, uuid.c_str());
	VALIDATE(written < 128, "Buffer overflow");

	return executeAndCheckQuery(query, SQLITE_DONE);
}

bool CachedPreviews::createCachedPreviewsDatabase(const std::string& filePath)
{
	int dbOpenResult = sqlite3_open_v2(filePath.c_str(), &_sqliteDatabase,
		SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

	if (dbOpenResult != SQLITE_OK)
		return false;

	return executeAndCheckQuery("CREATE TABLE PreviewsCache(uuid TEXT NOT NULL)", SQLITE_DONE);
}

bool CachedPreviews::executeAndCheckQuery(const char* query, int expectedResult) const
{
	CHECK(_sqliteDatabase);

	sqlite3_stmt* statement = nullptr;
	int statementResult = sqlite3_prepare_v2(_sqliteDatabase, query, -1, &statement,
		NULL);

	VALIDATE_AND_RETURN(nullptr, statementResult == SQLITE_OK, "Statement '%s' error. Reason: %s",
		query, sqlite3_errmsg(_sqliteDatabase));

	if (sqlite3_step(statement) == expectedResult)
		return true;

	return false;
}

std::string CachedPreviews::databaseFileName()
{
	return "previewscache.db";
}
} // lib
} // enlighten
