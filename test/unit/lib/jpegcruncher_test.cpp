#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "jpegcruncher.h"
#include "jpeg.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>

using namespace enlighten::lib;

MATCHER_P2(IsBetween, a, b, std::string(negation ? "isn't" : "is") + " between "
	+ testing::PrintToString(a) + " and " + testing::PrintToString(b)) { return a <= arg && arg <= b; }

class MockJpeg : public IJpeg
{
public:
	MOCK_METHOD0(decompress, bool());
	MOCK_METHOD1(compress, bool(uint32_t));

	MOCK_CONST_METHOD0(components, uint32_t());
	MOCK_CONST_METHOD0(width, uint32_t());
	MOCK_CONST_METHOD0(height, uint32_t());
	MOCK_CONST_METHOD0(rawBytes, const uint8_t*());

	MOCK_METHOD4(fromRawBytes, bool(uint8_t*,uint32_t,uint32_t,uint32_t));
	MOCK_CONST_METHOD1(compressedData, const uint8_t*(uint32_t&));

	MOCK_METHOD1(writeToFile, bool(const char*));
};

class JpegCruncherTest : public testing::Test
{
public:
	JpegCruncherTest()
	{
		uint32_t sourceW = 400;
		uint32_t sourceH = 200;
		uint32_t components = 3;
		size_t jpegRawSize = sourceW * sourceH * components;
		jpegBytes = (uint8_t*)malloc(jpegRawSize);
		memset(jpegBytes, 0, jpegRawSize);

		// Source jpeg
		ON_CALL(sourceJpeg, decompress())
			.WillByDefault(testing::Return(true));
		ON_CALL(sourceJpeg, rawBytes())
			.WillByDefault(testing::Return(jpegBytes));
		ON_CALL(sourceJpeg, width())
			.WillByDefault(testing::Return(sourceW));
		ON_CALL(sourceJpeg, height())
			.WillByDefault(testing::Return(sourceH));
		ON_CALL(sourceJpeg, components())
			.WillByDefault(testing::Return(components));

		// Target jpeg
		ON_CALL(targetJpeg, compress(testing::Gt(0)))
			.WillByDefault(testing::Return(true));
		ON_CALL(targetJpeg, fromRawBytes(testing::_, testing::Gt(0), testing::Gt(0), IsBetween(3,4)))
			.WillByDefault(testing::Return(true));
	}

	~JpegCruncherTest()
	{
		free(jpegBytes);
	}

protected:
	uint8_t* jpegBytes;

	MockJpeg sourceJpeg;
	MockJpeg targetJpeg;
};

TEST_F(JpegCruncherTest, ShouldReencodeToGivenDimensionAndQualityLevel)
{
	JpegCruncher cruncher(&sourceJpeg, &targetJpeg);

	EXPECT_CALL(sourceJpeg, decompress()).Times(1);
	EXPECT_CALL(sourceJpeg, width()).Times(testing::AtLeast(1));
	EXPECT_CALL(sourceJpeg, height()).Times(testing::AtLeast(1));
	EXPECT_CALL(sourceJpeg, components()).Times(testing::AtLeast(1));
	EXPECT_CALL(sourceJpeg, rawBytes()).Times(testing::AtLeast(1));

	EXPECT_CALL(targetJpeg, fromRawBytes(testing::_, 200, 100, 3));
	EXPECT_CALL(targetJpeg, compress(40));

	EXPECT_TRUE(cruncher.reencodeJpeg(200, 40));
}
