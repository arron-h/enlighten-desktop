#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "previewsdatabase.h"
#include "previewentry.h"
#include "cachedpreviews.h"

using namespace enlighten::lib;

static const char* PreviewsDatabase_ValidPreviewFile =
	"catalogs/Lightroom 5 Catalog Previews.lrdata/previews.db";

class MockCachedPreviews : public ICachedPreviews
{
public:
	MOCK_CONST_METHOD0(lastCachedTime, uint64_t());
	MOCK_CONST_METHOD0(numberOfCachedPreviews, uint32_t());

	MOCK_CONST_METHOD1(isInCache, bool(const enlighten::lib::uuid_t&));
	MOCK_METHOD1(markAsCached, void(const enlighten::lib::uuid_t&));
};

TEST(PreviewsDatabase, ShouldFailToInitialiseWithAnInvalidFile)
{
	PreviewsDatabase previews;
	EXPECT_FALSE(previews.initialiseWithFile("somefile.db"));
	EXPECT_FALSE(previews.initialiseWithFile(""));
}

TEST(PreviewsDatabase, ShouldInitialiseWithAValidFile)
{
	PreviewsDatabase previews;
	EXPECT_TRUE(previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile));
}

TEST(PreviewsDatabase, ShouldReopenAPreviouslyOpenedFile)
{
	PreviewsDatabase previews;
	EXPECT_TRUE(previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile));
	EXPECT_TRUE(previews.reopen());

	// TODO - Nice to add some modification of the source data here and verify.
}

TEST(PreviewsDatabase, ShouldFailToReopenIfNotOpenAlready)
{
	PreviewsDatabase previews;
	EXPECT_FALSE(previews.reopen());
}

TEST(PreviewsDatabase, ShouldReturnNumberOfPreviewEntries)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	EXPECT_EQ(3, previews.numberOfPreviewEntries());
}

TEST(PreviewsDatabase, ShouldReturnAUuidForAValidDatabaseIndex)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	enlighten::lib::uuid_t uuid;
	EXPECT_TRUE(previews.uuidForIndex(2, uuid));

	// TODO: Should we hardcode expectations for test data?
	EXPECT_EQ(uuid, "B089021B-7ACE-4A62-BD32-85A6C6AD5B9C");
}

TEST(PreviewsDatabase, ShouldFailWithAnInvalidDatabaseIndex)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	enlighten::lib::uuid_t uuid;
	EXPECT_FALSE(previews.uuidForIndex(99, uuid));
	EXPECT_TRUE(uuid.empty());
}

TEST(PreviewsDatabase, ShouldReturnAPreviewEntryForAValidUuid)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	enlighten::lib::uuid_t uuid = "B089021B-7ACE-4A62-BD32-85A6C6AD5B9C";

	const PreviewEntry* entry = previews.entryForUuid(uuid);
	ASSERT_TRUE(entry != nullptr);

	EXPECT_EQ(entry->digest(), "07cc63f155500a902b21fef7be6585b5");
}

TEST(PreviewsDatabase, ShouldReturnEntriesPastSpecificTime)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	std::set<enlighten::lib::uuid_t> entries;
	EXPECT_TRUE(previews.checkEntriesAgainstModificationTime(442191140, entries));

	EXPECT_EQ(2, entries.size());
}

TEST(PreviewsDatabase, ShouldReturnEntriesNotPresentInCache)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	MockCachedPreviews mockCache;

	EXPECT_CALL(mockCache, isInCache(testing::_))
		.Times(3)
		.WillOnce(testing::Return(true))
		.WillRepeatedly(testing::Return(false));

	std::set<enlighten::lib::uuid_t> entries;
	EXPECT_TRUE(previews.checkEntriesAgainstCachedPreviews(mockCache, entries));

	EXPECT_EQ(2, entries.size());
}
