#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "previewsdatabase.h"
#include "previewentry.h"

using namespace enlighten::lib;

static const char* PreviewsDatabase_ValidPreviewFile =
	"catalogs/Lightroom 5 Catalog Previews.lrdata/previews.db";

TEST(PreviewsDatabase, ShouldFailToInitialiseWithAnInvalidFile)
{
	PreviewsDatabase previews;
	EXPECT_FALSE(previews.initialiseWithFile("somefile.db"));
	EXPECT_FALSE(previews.initialiseWithFile(nullptr));
}

TEST(PreviewsDatabase, ShouldInitialiseWithAValidFile)
{
	PreviewsDatabase previews;
	EXPECT_TRUE(previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile));
}

TEST(PreviewsDatabase, ShouldReturnNumberOfPreviewEntries)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	EXPECT_EQ(3, previews.numberOfPreviewEntries());
}

TEST(PreviewsDatabase, ShouldReturnAPreviewEntryForAValidIndex)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	const PreviewEntry* entry = previews.entryForIndex(2);
	ASSERT_TRUE(entry != nullptr);

	// TODO: Should we hardcode expectations for test data?
	EXPECT_EQ("B089021B-7ACE-4A62-BD32-85A6C6AD5B9C", entry->uuid());
}

TEST(PreviewsDatabase, ShouldReturnNullWithAnInvalidPreviewIndex)
{
	PreviewsDatabase previews;
	previews.initialiseWithFile(PreviewsDatabase_ValidPreviewFile);

	const PreviewEntry* entry = previews.entryForIndex(99);
	EXPECT_TRUE(entry == nullptr);
}
