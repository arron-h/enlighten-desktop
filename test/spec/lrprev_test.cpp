#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "lrprev.h"

using namespace enlighten::lib;

static const char* LrPrev_ValidPreviewFiles[] =
{
	"catalogs/Lightroom 5 Catalog Previews.lrdata/3/3829/3829E5FC-7F3F-4B22-94F3-FB5E2C796026-07cc63f155500a902b21fef7be6585b5.lrprev",
	"catalogs/Lightroom 5 Catalog Previews.lrdata/6/6A2B/6A2B9912-3868-45E4-AE0D-7EA73F66FF63-07cc63f155500a902b21fef7be6585b5.lrprev",
	"catalogs/Lightroom 5 Catalog Previews.lrdata/B/B089/B089021B-7ACE-4A62-BD32-85A6C6AD5B9C-07cc63f155500a902b21fef7be6585b5.lrprev"
};

class LrPrevTest : public ::testing::TestWithParam<const char*>
{
};

TEST_F(LrPrevTest, ShouldFailToInitialiseWithAnInvalidFile)
{
	LrPrev lrPrev;
	EXPECT_FALSE(lrPrev.initialiseWithFile("somefile.lrprev"));
	EXPECT_FALSE(lrPrev.initialiseWithFile(NULL));
}

TEST_F(LrPrevTest, ShouldFailToInitialiseWhenFileIsNotAnLrPrevFile)
{
	LrPrev lrPrev;
	EXPECT_FALSE(lrPrev.initialiseWithFile("emptyfile.lrprev"));
}

TEST_F(LrPrevTest, ShouldInitialiseWithAValidFile)
{
	LrPrev lrPrev;
	EXPECT_TRUE(lrPrev.initialiseWithFile(LrPrev_ValidPreviewFiles[0]));
}

TEST_P(LrPrevTest, ShouldExtractJpegFromAValidLevel)
{
	LrPrev lrPrev;
	lrPrev.initialiseWithFile(GetParam());

	unsigned int numberOfJpegBytes = 0;
	unsigned char* bytes = lrPrev.extractFromLevel(1, numberOfJpegBytes);

	ASSERT_TRUE(bytes != NULL);
	ASSERT_GT(numberOfJpegBytes, 2);

	/*
	 Do a quick sanity check that the Jpeg marker bytes
	 are correct. We should probably check against a GM
	 image or something.
	 */

	// Start of Image
	EXPECT_EQ(0xFF, bytes[0]);
	EXPECT_EQ(0xD8, bytes[1]);

	// End of Image
	EXPECT_EQ(0xFF, bytes[numberOfJpegBytes-2]);
	EXPECT_EQ(0xD9, bytes[numberOfJpegBytes-1]);

	free(bytes);
}

TEST_F(LrPrevTest, ShouldReturnNullIfLevelDoesNotExist)
{
	LrPrev lrPrev;
	lrPrev.initialiseWithFile(LrPrev_ValidPreviewFiles[0]);

	unsigned int numberOfJpegBytes = 0;
	EXPECT_TRUE(lrPrev.extractFromLevel(9, numberOfJpegBytes) == NULL);
	EXPECT_EQ(0, numberOfJpegBytes);
}

INSTANTIATE_TEST_CASE_P( , LrPrevTest, ::testing::ValuesIn(LrPrev_ValidPreviewFiles));
