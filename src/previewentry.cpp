#include "previewentry.h"

#include "validation.h"

const unsigned int PreviewEntry::INVALID_LEVEL_INDEX = 0xFFFF;

PreviewEntry::PreviewEntry()
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
	return std::string("");
}

unsigned int PreviewEntry::numberOfLevels() const
{
	return 0;
}

unsigned int PreviewEntry::closestLevelIndex(unsigned int longDimension) const
{
	return INVALID_LEVEL_INDEX;
}
