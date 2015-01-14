#ifndef PREVIEW_ENTRY_H
#define PREVIEW_ENTRY_H

#include <string>
#include <vector>

#include "previewentrylevel.h"

namespace enlighten
{
namespace lib
{
typedef std::string uuid_t;

class PreviewEntry
{
public:
	PreviewEntry(const std::string& uuid, const std::string& digest,
		const std::vector<PreviewEntryLevel>& levels);

	const uuid_t& uuid() const;
	const std::string& digest() const;

	std::string filePathRelativeToRoot() const;

	unsigned int numberOfLevels() const;
	unsigned int closestLevelToDimension(float longDimension) const;

	static const unsigned int INVALID_LEVEL_INDEX;

private:
	uuid_t _uuid;
	std::string _digest;

	std::vector<PreviewEntryLevel> _levels;
};
} // lib
} // enlighten

#endif // PREVIEW_ENTRY_H
