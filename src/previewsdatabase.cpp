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
}

bool PreviewsDatabase::initialiseWithFile(const char* fileName)
{
	VALIDATE(fileName, "fileName argument is invalid");

	int dbOpenResult = sqlite3_open_v2(fileName, &_sqliteDatabase, SQLITE_OPEN_READONLY, NULL);
	VALIDATE(dbOpenResult == SQLITE_OK, "Failed to open sqlite database '%s'. Reason: %s",
		fileName, sqlite3_errmsg(_sqliteDatabase));

	return true;
}

unsigned int PreviewsDatabase::numberOfPreviewEntries()
{
	return 0;
}

const PreviewEntry* PreviewsDatabase::entryForIndex(unsigned int index)
{
	return NULL;
}
} // lib
} // enlighten