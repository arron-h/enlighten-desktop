#include "previewentry.h"

#include "validation.h"

namespace enlighten
{
namespace lib
{
const unsigned int PreviewEntry::INVALID_LEVEL_INDEX = 0xFFFF;

PreviewEntry::PreviewEntry(const std::string& uuid, const std::string& digest,
	const std::vector<PreviewEntryLevel>& levels) : _uuid(uuid), _digest(digest),
		_levels(levels)
{
}

const std::string& PreviewEntry::uuid() const
{
	return _uuid;
}

const std::string& PreviewEntry::digest() const
{
	return _digest;
}

std::string PreviewEntry::filePathRelativeToRoot() const
{
	char buffer[96];
	int written = snprintf(buffer, 96, "%s/%s/%s-%s.lrprev", _uuid.substr(0,1).c_str(),
		_uuid.substr(0,4).c_str(), _uuid.c_str(), _digest.c_str());

	VALIDATE_AND_RETURN(std::string(""), written < 96, "Buffer overflow");

	return std::string(buffer);
}

unsigned int PreviewEntry::numberOfLevels() const
{
	return _levels.size();
}

unsigned int PreviewEntry::closestLevelToDimension(float longDimension) const
{
	for (unsigned int levelIdx = 0; levelIdx < numberOfLevels(); ++levelIdx)
	{
		const PreviewEntryLevel& level = _levels[levelIdx];
		if (longDimension < level.longDimension())
			return level.levelNumber();
	}

	return INVALID_LEVEL_INDEX;
}
} // lib
} // enlighten
