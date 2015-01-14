#ifndef CACHED_PREVIEWS_H
#define CACHED_PREVIEWS_H

#include "previewentry.h"

namespace enlighten
{
namespace lib
{
class ICachedPreviews
{
public:
	virtual ~ICachedPreviews() {}

	virtual uint64_t lastCachedTime() const = 0;
	virtual uint32_t numberOfCachedPreviews() const = 0;

	virtual bool isInCache(const uuid_t& uuid) const = 0;
	virtual void markAsCached(const uuid_t& uuid) = 0;
};

class CachedPreviews : public ICachedPreviews
{
public:
	CachedPreviews();

	uint64_t lastCachedTime() const;
	uint32_t numberOfCachedPreviews() const;

	bool isInCache(const uuid_t& uuid) const;
	void markAsCached(const uuid_t& uuid);
};
} // lib
} // enlighten
#endif // CACHED_PREVIEWS_H
