#ifndef PREVIEW_ENTRY_LEVEL_H
#define PREVIEW_ENTRY_LEVEL_H

namespace enlighten
{
namespace lib
{
class PreviewEntryLevel
{
public:
	PreviewEntryLevel(unsigned int levelNumber, float longDimension);

	unsigned int levelNumber() const;
	float longDimension() const;

private:
	unsigned int _levelNumber;
	float _longDimension;
};
} // lib
} // enlighten

#endif // PREVIEW_ENTRY_LEVEL_H
