#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "cachedpreviews.h"
#include "settings.h"
#include "file.h"

#include <chrono>

using namespace enlighten::lib;

namespace
{
	std::string CachedPreviewsTest_PathToCachedPreviewsRoot = "temp/";

	class MockSettings : public IStaticSettings
	{
	public:
		MOCK_CONST_METHOD1(get, SettingValue(settings::StaticSetting));
	};

	class CachedPreviewsTest : public testing::Test
	{
	public:
		CachedPreviewsTest()
		{
			ON_CALL(settings, get(settings::CachedDatabasePath))
				.WillByDefault(testing::Return(
					SettingValue(&CachedPreviewsTest_PathToCachedPreviewsRoot)));
		}

		~CachedPreviewsTest()
		{
			File f(CachedPreviewsTest_PathToCachedPreviewsRoot + CachedPreviews::databaseFileName());
			if (f.isValid())
				EXPECT_TRUE(f.remove());
		}

		MockSettings settings;
	};
}

TEST_F(CachedPreviewsTest, ShouldConstructWithSettings)
{
	CachedPreviews previews(&settings);
}

TEST_F(CachedPreviewsTest, ShouldCreateDatabaseWhenOneDoesNotExist)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	File file(CachedPreviewsTest_PathToCachedPreviewsRoot + CachedPreviews::databaseFileName());
	EXPECT_FALSE(file.isValid());

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());

	EXPECT_TRUE(file.isValid());
}

TEST_F(CachedPreviewsTest, ShouldLoadDatabaseWhenExists)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	File file(CachedPreviewsTest_PathToCachedPreviewsRoot + CachedPreviews::databaseFileName());
	EXPECT_FALSE(file.isValid());
	{
		CachedPreviews previews(&settings);
		EXPECT_TRUE(previews.loadOrCreateDatabase()); // Create a database
	}

	EXPECT_TRUE(file.isValid());
	{
		CachedPreviews previews(&settings);
		EXPECT_TRUE(previews.loadOrCreateDatabase());
	}
}

TEST_F(CachedPreviewsTest, ShouldMarkPreviewAsCached)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	enlighten::lib::uuid_t uuid = "123456-789ABCDEF";

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());
	EXPECT_TRUE(previews.markAsCached(uuid));
}

TEST_F(CachedPreviewsTest, ShouldReturnTrueIfPreviewIsAlreadyCached)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	enlighten::lib::uuid_t uuid = "123456-789ABCDEF";

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());
	EXPECT_TRUE(previews.markAsCached(uuid));

	EXPECT_TRUE(previews.isInCache(uuid));
}

TEST_F(CachedPreviewsTest, ShouldReturnFalseIfPreviewNotInCache)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	enlighten::lib::uuid_t uuid = "123456-789ABCDEF";

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());

	EXPECT_FALSE(previews.isInCache(uuid));
}

TEST_F(CachedPreviewsTest, ShouldCacheManyUuids)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	enlighten::lib::uuid_t uuid1 = "123456-789ABCDEF";
	enlighten::lib::uuid_t uuid2 = "56789A-789ABCDEF";
	enlighten::lib::uuid_t uuid3 = "FEDCBA-12345ADBE";
	enlighten::lib::uuid_t uuid4 = "ABCDEF-123456789"; // not in cache

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());
	EXPECT_TRUE(previews.markAsCached(uuid1));
	EXPECT_TRUE(previews.markAsCached(uuid2));
	EXPECT_TRUE(previews.markAsCached(uuid3));

	EXPECT_TRUE(previews.isInCache(uuid1));
	EXPECT_TRUE(previews.isInCache(uuid2));
	EXPECT_TRUE(previews.isInCache(uuid3));
	EXPECT_FALSE(previews.isInCache(uuid4));
}

TEST_F(CachedPreviewsTest, ShouldReturnNumberOfCachedPreviews)
{
	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	enlighten::lib::uuid_t uuid1 = "123456-789ABCDEF";
	enlighten::lib::uuid_t uuid2 = "56789A-789ABCDEF";
	enlighten::lib::uuid_t uuid3 = "FEDCBA-12345ADBE";

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());
	EXPECT_TRUE(previews.markAsCached(uuid1));
	EXPECT_TRUE(previews.markAsCached(uuid2));
	EXPECT_TRUE(previews.markAsCached(uuid3));

	EXPECT_EQ(3, previews.numberOfCachedPreviews());
}

TEST_F(CachedPreviewsTest, ShouldGenerateProxySetForCachedEntries)
{
	std::set<enlighten::lib::uuid_t> entries;

	EXPECT_CALL(settings, get(settings::CachedDatabasePath))
			.WillRepeatedly(testing::DoDefault());

	enlighten::lib::uuid_t uuid1 = "123456-789ABCDEF";
	enlighten::lib::uuid_t uuid2 = "56789A-789ABCDEF";

	CachedPreviews previews(&settings);
	EXPECT_TRUE(previews.loadOrCreateDatabase());
	EXPECT_TRUE(previews.markAsCached(uuid1));
	EXPECT_TRUE(previews.markAsCached(uuid2));

	previews.generateProxy(entries);

	EXPECT_EQ(2, entries.size());
}
