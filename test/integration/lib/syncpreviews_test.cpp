#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "synchronizers/previewssynchronizer.h"
#include "cachedpreviews.h"
#include "settings.h"
#include "file.h"
#include "aws/aws.h"

#include <cstdio>
#include <sys/stat.h>
#include <thread>

using namespace enlighten::lib;

namespace
{
	const char* SyncPreviewsTest_PreviewsDatabase = "catalogs/Lightroom 5 Catalog Previews.lrdata/previews.db";

	class SyncPreviewsTest : public testing::Test
	{
	public:
		SyncPreviewsTest()
		{
			// Purge the cache file
			File f(CachedPreviews::databaseFileName());
			f.remove();

			// Duplicate the test database, to make these tests standalone
			File duplicatePreviews(SyncPreviewsTest_PreviewsDatabase);
			EXPECT_TRUE(duplicatePreviews.duplicate(databaseFileName));

			AwsDestination awsDest = { "my-bucket", "" };
			AwsAccessProfile accessProfile = { "12345", "67890" };
			AwsConfig config;
			config.hostName = "http://localhost:5678";

			Aws& aws = Aws::get();
			aws.initialise(config);
			aws.initialiseDestinationWithProfile("testprofile", accessProfile, awsDest);
			synchronizer = new PreviewsSynchronizer(&settings, &aws);
		}

		~SyncPreviewsTest()
		{
			File duplicatePreviews(databaseFileName);
			EXPECT_TRUE(duplicatePreviews.remove());

			delete synchronizer;

			Aws::get().shutdown();
		}

		const char* databaseFileName = "temp/SyncPreviewsTest_previews.db";
		PreviewsSynchronizer* synchronizer;
		EnlightenSettings settings;
	};
}

TEST_F(SyncPreviewsTest, KeepPreviewsSynchronized)
{
	EXPECT_TRUE(synchronizer->beginSynchronizingFile(SyncPreviewsTest_PreviewsDatabase, "testprofile"));

	// Wait a little
	std::this_thread::sleep_for(std::chrono::seconds(10));

	// Modify the copy database
}

