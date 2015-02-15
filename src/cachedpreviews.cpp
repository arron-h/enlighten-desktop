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
CachedPreviews::CachedPreviews(IStaticSettings* settings) : _sqliteDatabase(nullptr),
	_settings(settings)
{
}

CachedPreviews::~CachedPreviews()
{
	if (_sqliteDatabase)
		sqlite3_close(_sqliteDatabase);
}

bool CachedPreviews::loadOrCreateDatabase()
{
	std::string filePath = _settings->get(settings::CachedDatabasePath).toString();
	filePath += databaseFileName();

	int dbOpenResult = sqlite3_open_v2(filePath.c_str(), &_sqliteDatabase,
		SQLITE_OPEN_READWRITE, NULL);

	if (dbOpenResult != SQLITE_OK)
		return createCachedPreviewsDatabase(filePath);

	return true;
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

	VALIDATE(executeAndCheckQuery(query, SQLITE_ROW), "Sql result != SQLITE_DONE");

	return true;
}

bool CachedPreviews::markAsCached(const uuid_t& uuid)
{
	VALIDATE(_sqliteDatabase, "Sqlite database is in an invalid state.");

	const char* queryFormat = "INSERT INTO PreviewsCache (uuid) VALUES "
                               "('%s')";

	char query[128];
	int written = snprintf(query, 128, queryFormat, uuid.c_str());
	VALIDATE(written < 128, "Buffer overflow");

	VALIDATE(executeAndCheckQuery(query, SQLITE_DONE), "Sql result != SQLITE_DONE");

	return true;
}

bool CachedPreviews::generateProxy(std::set<uuid_t>& entries) const
{
	// Iterates over all of the current cached entries and returns a
	// 'proxy' container, which is mutable by clients.

	VALIDATE(_sqliteDatabase, "Sqlite database is in an invalid state.");

	const char* query = "SELECT uuid FROM PreviewsCache";

	sqlite3_stmt* statement = nullptr;
	int statementResult = sqlite3_prepare_v2(_sqliteDatabase, query, -1, &statement,
		NULL);

	VALIDATE_AND_RETURN(nullptr, statementResult == SQLITE_OK, "Statement '%s' error. Reason: %s",
		query, sqlite3_errmsg(_sqliteDatabase));

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		const char* uuidStr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
		entries.insert(uuidStr);
	}

	return true;
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
