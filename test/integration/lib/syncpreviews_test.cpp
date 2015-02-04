#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "synchronizers/previewssynchronizer.h"
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
			// Duplicate the test database, to make these tests standalone
			File duplicatePreviews(SyncPreviewsTest_PreviewsDatabase);
			EXPECT_TRUE(duplicatePreviews.duplicate(databaseFileName));

			AwsDestination awsDest = { "my-bucket", "" };
			AwsAccessProfile accessProfile = { "12345", "67890" };

			Aws aws = Aws::get();
			aws.initialiseDestinationWithProfile("testprofile", accessProfile, awsDest);
			synchronizer = new PreviewsSynchronizer(&settings, &aws);
		}

		~SyncPreviewsTest()
		{
			File duplicatePreviews(databaseFileName);
			EXPECT_TRUE(duplicatePreviews.remove());

			delete synchronizer;
		}

		const char* databaseFileName = "temp/SyncPreviewsTest_previews.db";
		PreviewsSynchronizer* synchronizer;
		EnlightenSettings settings;
	};
}

TEST_F(SyncPreviewsTest, KeepPreviewsSynchronized)
{
	EXPECT_TRUE(synchronizer->beginSynchronizingFile(databaseFileName, "testprofile"));

	// Wait a little
	std::this_thread::sleep_for(std::chrono::nanoseconds(1000 * 1000 * 2));

	// Modify the copy database
}

