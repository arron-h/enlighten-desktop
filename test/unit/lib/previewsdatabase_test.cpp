#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "previewsdatabase.h"
#include "previewentry.h"
#include "cachedpreviews.h"

using namespace enlighten::lib;

namespace
{
	static const char* PreviewsDatabase_ValidPreviewFile =
		"catalogs/Lightroom 5 Catalog Previews.lrdata/previews.db";

	class MockCachedPreviews : public ICachedPreviews
	{
	public:
		MOCK_CONST_METHOD0(numberOfCachedPreviews, uint32_t());

		MOCK_CONST_METHOD1(generateProxy, bool(std::set<enlighten::lib::uuid_t>&));
		MOCK_CONST_METHOD1(isInCache, bool(const enlighten::lib::uuid_t&));
		MOCK_METHOD1(markAsCached, bool(const enlighten::lib::uuid_t&));
	};

	bool buildMockCache_AddAction(std::set<enlighten::lib::uuid_t>& uuids)
	{
		// Mock only 1 uuid in the cache
		uuids.insert("3829E5FC-7F3F-4B22-94F3-FB5E2C796026");
	}

	bool buildMockCache_RemoveAction(std::set<enlighten::lib::uuid_t>& uuids)
	{
		// These uuids exist in the test data and will be ignored
		uuids.insert("3829E5FC-7F3F-4B22-94F3-FB5E2C796026");
		uuids.insert("6A2B9912-3868-45E4-AE0D-7EA73F66FF63");
		uuids.insert("B089021B-7ACE-4A62-BD32-85A6C6AD5B9C");

		// Mock entries in the cache which don't exist in the test data - this
		// will trigger 'removals'
		uuids.insert("3829E5FC");
		uuids.insert("ABCDEF12");
		uuids.insert("3456789A");

		return true;
	}
}

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

TEST(PreviewsDatabase, ShouldReturnNewEntriesWithAddAction)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	MockCachedPreviews mockCache;

	EXPECT_CALL(mockCache, generateProxy(testing::_))
		.Times(1)
		.WillOnce(testing::Invoke(buildMockCache_AddAction));

	std::map<enlighten::lib::uuid_t, SyncAction> entries;
	EXPECT_TRUE(previews.checkEntriesAgainstCachedPreviews(mockCache, entries));

	EXPECT_EQ(2, entries.size());
	for(auto entry : entries)
	{
		EXPECT_EQ(SyncAction_Add, entry.second);
	}
}

TEST(PreviewsDatabase, ShouldReturnNonExistentOldEntriesWithRemoveAction)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	MockCachedPreviews mockCache;

	EXPECT_CALL(mockCache, generateProxy(testing::_))
		.Times(1)
		.WillOnce(testing::Invoke(buildMockCache_RemoveAction));

	std::map<enlighten::lib::uuid_t, SyncAction> entries;
	EXPECT_TRUE(previews.checkEntriesAgainstCachedPreviews(mockCache, entries));

	EXPECT_EQ(3, entries.size());
	for(auto entry : entries)
	{
		EXPECT_EQ(SyncAction_Remove, entry.second);
	}
}
