#ifndef PREVIEW_ENTRY_H
#define PREVIEW_ENTRY_H

#include <string>
#include <vector>

class PreviewEntry
{
public:
	PreviewEntry();

	const std::string& uuid() const;
	const std::string& digest() const;

	std::string filePathRelativeToRoot() const;

	unsigned int numberOfLevels() const;
	unsigned int closestLevelIndex(unsigned int longDimension) const;

	static const unsigned int INVALID_LEVEL_INDEX;

private:
	std::string _uuid;
	std::string _digest;

	struct Level
	{
		unsigned int levelNumber;
		float longDimension;
	};
	std::vector<Level> _levels;

	friend class PreviewDatabase;
};

#endif // PREVIEW_ENTRY_H
