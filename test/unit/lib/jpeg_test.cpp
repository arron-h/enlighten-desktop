#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#include "jpeg.h"

using namespace enlighten::lib;

namespace
{
	class JpegTest : public testing::Test
	{
	public:
		JpegTest() : jpegBytes(nullptr), byteSize(0), rgbData(nullptr)
		{
		}

		~JpegTest()
		{
			if (jpegBytes)
			{
				free(jpegBytes);
				jpegBytes = nullptr;
			}

			if (rgbData)
			{
				free(rgbData);
				rgbData = nullptr;
			}
		}

		void loadTestAsset()
		{
			FILE* file = fopen("lena.jpg", "rb");
			ASSERT_TRUE(file != NULL);

			struct stat buf;
			fstat(fileno(file), &buf);
			byteSize = buf.st_size;

			jpegBytes = (uint8_t*)malloc(byteSize);
			ASSERT_TRUE(jpegBytes);

			fread(jpegBytes, 1, byteSize, file);
			fclose(file);
		}

		void generateTestRgba()
		{
			rgbWidth  = 256;
			rgbHeight = 256;
			rgbData = (uint8_t*)malloc(rgbWidth*rgbHeight*3);

			for (int y = 0; y < rgbHeight; ++y)
			{
				for (int x = 0; x < rgbWidth; ++x)
				{
					uint32_t index = (y * (rgbWidth * 3)) + x * 3;

					// Fills the data with a fancy pattern
					uint32_t col = (255<<24) + ((rgbHeight-x)<<16) + (x<<8) + y;
					if ((x*y)/8 % 2) col = 0;

					memcpy(rgbData + index, &col, 3);
				}
			}
		}

		uint8_t* jpegBytes;
		uint32_t byteSize;

		uint32_t rgbWidth;
		uint32_t rgbHeight;
		uint8_t* rgbData;
	};
}

TEST_F(JpegTest, ShouldConstructAndRetainBytes)
{
	uint8_t bytes[] = { 0xFE,0xA1,0x43,0x61,0xAC,0x1D,0xCA,0xFE };
	Jpeg jpeg(bytes, sizeof(bytes), true);

	uint32_t compressedDataSize;
	EXPECT_TRUE(bytes != jpeg.compressedData(compressedDataSize));
	EXPECT_EQ(compressedDataSize, sizeof(bytes));

	memset(bytes, 0x0, sizeof(bytes));
	EXPECT_EQ(0xFE, jpeg.compressedData(compressedDataSize)[0]);
}

TEST_F(JpegTest, ShouldConstructAndNotRetainBytes)
{
	uint8_t bytes[] = { 0xFE,0xA1,0x43,0x61,0xAC,0x1D,0xCA,0xFE };
	Jpeg jpeg(bytes, sizeof(bytes), false);

	uint32_t compressedDataSize;
	EXPECT_EQ(jpeg.compressedData(compressedDataSize), bytes);
	EXPECT_EQ(compressedDataSize, sizeof(bytes));

	memset(bytes, 0x0, sizeof(bytes));
	EXPECT_EQ(0x0, jpeg.compressedData(compressedDataSize)[0]);
}

TEST_F(JpegTest, ShouldDecompressAJpeg)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.decompress());
	EXPECT_TRUE(jpeg.rawBytes() != nullptr);

	EXPECT_EQ(512, jpeg.width());
	EXPECT_EQ(512, jpeg.height());
}

TEST_F(JpegTest, ShouldFailDecompressWhenNoSourceSet)
{
	Jpeg jpeg;
	EXPECT_FALSE(jpeg.decompress());
}

TEST_F(JpegTest, ShouldFailDecompressWhenDecompressedImageAlreadyInMemory)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);
	EXPECT_TRUE(jpeg.decompress());
	EXPECT_FALSE(jpeg.decompress());
}

TEST_F(JpegTest, ShouldFailDecompressWhenRawBytesAlreadySet)
{
	loadTestAsset();
	generateTestRgba();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.fromRawBytes(rgbData, rgbWidth, rgbHeight, 3));
	EXPECT_FALSE(jpeg.decompress());
}

TEST_F(JpegTest, ShouldCompressFromRawBytes)
{
	generateTestRgba();

	Jpeg jpeg;
	jpeg.fromRawBytes(rgbData, rgbWidth, rgbHeight, 3);

	EXPECT_TRUE(jpeg.compress(40));

	uint32_t compressedDataSize;
	EXPECT_TRUE(jpeg.compressedData(compressedDataSize) != nullptr);
	EXPECT_GT(compressedDataSize, 0);
}

TEST_F(JpegTest, ShouldFailCompressWhenNoRGBADataSet)
{
	Jpeg jpeg;
	EXPECT_FALSE(jpeg.compress(60));
}

TEST_F(JpegTest, ShouldFailCompressWithInvalidQualityLevel)
{
	generateTestRgba();

	Jpeg jpeg;
	jpeg.fromRawBytes(rgbData, rgbWidth, rgbHeight, 3);

	EXPECT_FALSE(jpeg.compress(94353));
}

TEST_F(JpegTest, ShouldFailCompressWhenContainsDecompressedData)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.decompress());
	EXPECT_FALSE(jpeg.compress(50));
}

TEST_F(JpegTest, ShouldReturnNumberOfComponents)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.decompress());
	EXPECT_EQ(3, jpeg.components());
}

TEST_F(JpegTest, ShouldReturnWidth)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.decompress());
	EXPECT_EQ(512, jpeg.width());
}

TEST_F(JpegTest, ShouldReturnHeight)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.decompress());
	EXPECT_EQ(512, jpeg.height());
}

TEST_F(JpegTest, ShouldReturnRawBytes)
{
	loadTestAsset();

	Jpeg jpeg(jpegBytes, byteSize, false);

	EXPECT_TRUE(jpeg.decompress());
	EXPECT_TRUE(jpeg.rawBytes() != nullptr);
}

TEST_F(JpegTest, DISABLED_ShouldReplaceDecompressedBytesWithRawBytesAndNotLeak)
{
	// We can't track this without some allocation tracking.
}

TEST_F(JpegTest, ShouldFailToCreateFromRawBytesWithInvalidArguments)
{
	generateTestRgba();
	Jpeg jpeg;

	EXPECT_FALSE(jpeg.fromRawBytes(nullptr, 512, 512, 3));
	EXPECT_FALSE(jpeg.fromRawBytes(rgbData, 0, 512, 3));
	EXPECT_FALSE(jpeg.fromRawBytes(rgbData, 512, 0, 3));
	EXPECT_FALSE(jpeg.fromRawBytes(rgbData, 512, 512, 10));
}

TEST_F(JpegTest, ShouldWriteCompressedJpegToFile)
{
	generateTestRgba();
	Jpeg jpeg;

	EXPECT_TRUE(jpeg.fromRawBytes(rgbData, rgbWidth, rgbHeight, 3));
	EXPECT_TRUE(jpeg.compress(100));

	EXPECT_TRUE(jpeg.writeToFile("temp/JpegTest_ShouldWriteCompressedJpegToFile.jpg"));
}
