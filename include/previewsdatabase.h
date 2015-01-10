#ifndef PREVIEWS_DATABASE_H
#define PREVIEWS_DATABASE_H

#include <vector>

struct sqlite3;

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
	std::vector<PreviewEntry*> _cachedEntries;
	sqlite3* _sqliteDatabase;
};
} // lib
} // enlighten

#endif // PREVIEWS_DATABASE_H
