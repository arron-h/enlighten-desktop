#include "previewentrylevel.h"

namespace enlighten
{
namespace lib
{
PreviewEntryLevel::PreviewEntryLevel(unsigned int levelNumber, float longDimension) :
	_levelNumber(levelNumber), _longDimension(longDimension)
{
}

unsigned int PreviewEntryLevel::levelNumber() const
{
	return _levelNumber;
}

float PreviewEntryLevel::longDimension() const
{
	return _longDimension;
}
} // lib
} // enlighten
