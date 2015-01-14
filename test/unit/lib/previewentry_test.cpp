#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "previewentry.h"

using namespace enlighten::lib;

class PreviewEntryTest : public testing::Test
{
public:
	PreviewEntryTest()
	{
		fakeUuid   = "3829E5FC-7F3F-4B22-94F3-FB5E2C796026";
		fakeDigest = "07cc63f155500a902b21fef7be6585b5";

		fakeLevels.push_back(PreviewEntryLevel(1, 67.0f));
		fakeLevels.push_back(PreviewEntryLevel(2, 134.0f));
		fakeLevels.push_back(PreviewEntryLevel(3, 543.0f));
		fakeLevels.push_back(PreviewEntryLevel(4, 1068.0f));
	}

protected:
	std::string fakeUuid;
	std::string fakeDigest;
	std::vector<PreviewEntryLevel> fakeLevels;
};

TEST_F(PreviewEntryTest, ShouldReturnNumberOfLevelsAvailable)
{
	PreviewEntry entry(fakeUuid, fakeDigest, fakeLevels);
	unsigned int numberOfLevels = entry.numberOfLevels();

	EXPECT_EQ(4, numberOfLevels);
}

TEST_F(PreviewEntryTest, ShouldPrepareAValidFilepath)
{
	PreviewEntry entry(fakeUuid, fakeDigest, fakeLevels);
	std::string generatedPath = entry.filePathRelativeToRoot();

	EXPECT_STREQ("3/3829/3829E5FC-7F3F-4B22-94F3-FB5E2C796026-"
		"07cc63f155500a902b21fef7be6585b5.lrprev", generatedPath.c_str());
}

TEST_F(PreviewEntryTest, ShouldReturnClosestLevelToAGivenDimension)
{
	PreviewEntry entry(fakeUuid, fakeDigest, fakeLevels);
	unsigned int level = entry.closestLevelToDimension(256.0f);

	EXPECT_EQ(3, level);
}

TEST_F(PreviewEntryTest, ShouldReturnAnInvalidLevelIndexIfNoSuitableDimensionExists)
{
	PreviewEntry entry(fakeUuid, fakeDigest, fakeLevels);
	unsigned int level = entry.closestLevelToDimension(2048.0f);

	EXPECT_EQ(PreviewEntry::INVALID_LEVEL_INDEX, level);
}
