#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "lrprev.h"
#include "jpegcruncher.h"
#include "jpeg.h"

#include <cstdio>
#include <sys/stat.h>

using namespace enlighten::lib;

namespace
{
	const char* lrPrevFile = "catalogs/Lightroom 5 Catalog Previews.lrdata/3/3829/3829E5FC-7F3F-4B22-94F3-FB5E2C796026-07cc63f155500a902b21fef7be6585b5.lrprev";
}

TEST(JpegPipelineTest, ProcessJpegFromLrCat)
{
	const char* destinationFile = "temp/JpegPipelineTest_ProcessJpegFromLrCat.jpg";

	LrPrev lrprev;
	ASSERT_TRUE(lrprev.initialiseWithFile(lrPrevFile));

	uint32_t dataSize;
	uint8_t* bytes = lrprev.extractFromLevel(3, dataSize);

	ASSERT_TRUE(bytes != nullptr);
	ASSERT_TRUE(dataSize != 0);

	Jpeg sourceJpeg(bytes, dataSize, false);
	Jpeg targetJpeg;

	JpegCruncher cruncher(&sourceJpeg, &targetJpeg);
	EXPECT_TRUE(cruncher.reencodeJpeg(200, 40));

	EXPECT_TRUE(targetJpeg.writeToFile(destinationFile));
}

