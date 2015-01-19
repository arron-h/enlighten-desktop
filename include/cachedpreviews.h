#ifndef CACHED_PREVIEWS_H
#define CACHED_PREVIEWS_H

#include "previewentry.h"
#include <set>

struct sqlite3;
struct sqlite3_stmt;

namespace enlighten
{
namespace lib
{
class IEnlightenSettings;
class ICachedPreviews
{
public:
	virtual ~ICachedPreviews() {}

	virtual uint32_t numberOfCachedPreviews() const = 0;

	virtual bool generateProxy(std::set<uuid_t>& entries) const = 0;
	virtual bool isInCache(const uuid_t& uuid) const = 0;
	virtual bool markAsCached(const uuid_t& uuid) = 0;
};

class CachedPreviews : public ICachedPreviews
{
public:
	CachedPreviews(IEnlightenSettings* settings);
	~CachedPreviews();

	bool loadOrCreateDatabase();

	uint32_t numberOfCachedPreviews() const;

	bool generateProxy(std::set<uuid_t>& entries) const;
	bool isInCache(const uuid_t& uuid) const;
	bool markAsCached(const uuid_t& uuid);

	static std::string databaseFileName();

private:
	bool createCachedPreviewsDatabase(const std::string& filePath);
	bool executeAndCheckQuery(const char* query, int expectedResult) const;

private:
	sqlite3* _sqliteDatabase;
	IEnlightenSettings* _settings;
};
} // lib
} // enlighten
#endif // CACHED_PREVIEWS_H
