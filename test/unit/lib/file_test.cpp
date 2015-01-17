#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstdio>
#include <cstring>
#include <chrono>

#include "file.h"

using namespace enlighten::lib;

namespace
{
	const char* FileTest_TestSource = "lena.jpg";

	template <int TSize>
	void writeTestFile(const char* filePath)
	{
		File file(filePath);

		uint8_t buffer[TSize];
		memset(buffer, 0x0, TSize);
		EXPECT_TRUE(file.openWrite());
		EXPECT_EQ(TSize, file.write(buffer, TSize));
		file.close();
	}
}

TEST(FileTest, ShouldConstructWithFilePath)
{
	File file(FileTest_TestSource);
}

TEST(FileTest, ShouldReturnFilePathFromConstruction)
{
	File file(FileTest_TestSource);
	EXPECT_STREQ(FileTest_TestSource, file.filePath());
}

TEST(FileTest, ShouldOpenAValidFileForReading)
{
	File file(FileTest_TestSource);
	EXPECT_TRUE(file.openRead());
}

TEST(FileTest, ShouldFailOpenAnInvalidFileForReading)
{
	File file("none_existant_file.dat");
	EXPECT_FALSE(file.openRead());
}

TEST(FileTest, ShouldOpenAFileForWriting)
{
	File file("temp/open_write_file_test.dat");
	EXPECT_TRUE(file.openWrite());
}

TEST(FileTest, ShouldFailOpenForReadingWhenAlreadyOpenedForWriting)
{
	File file("temp/open_write_file_test.dat");
	EXPECT_TRUE(file.openWrite());
	EXPECT_FALSE(file.openRead());
}

TEST(FileTest, ShouldFailOpenForWritingWhenAlreadyOpenedForReading)
{
	File file(FileTest_TestSource);
	EXPECT_TRUE(file.openRead());
	EXPECT_FALSE(file.openWrite());
}

TEST(FileTest, ShouldReadToGivenBuffer)
{
	uint8_t buffer[1024];

	File file(FileTest_TestSource);
	EXPECT_TRUE(file.openRead());
	EXPECT_EQ(1024, file.read(buffer, 1024));
}

TEST(FileTest, ShouldWriteForGivenBuffer)
{
	uint8_t buffer[1024];
	memset(buffer, 0x0, 1024);

	File file("temp/write_file_test.dat");
	EXPECT_TRUE(file.openWrite());
	EXPECT_EQ(1024, file.write(buffer, 1024));
}

TEST(FileTest, ShouldReturnFileSize)
{
	const char* filename = "temp/file_size_test.dat";
	writeTestFile<512>(filename);

	File file(filename);
	EXPECT_EQ(512, file.fileSize());
}

TEST(FileTest, ShouldReturnTrueWhenFilePassedIsValid)
{
	File file(FileTest_TestSource);
	EXPECT_TRUE(file.isValid());
}

TEST(FileTest, ShouldReturnFalseWhenFilePassedIsInvalid)
{
	File file("somefilethatdoesnotexist.dat");
	EXPECT_FALSE(file.isValid());
}

TEST(FileTest, ShouldDuplicateAValidFile)
{
	const char* duplicateFilePath = "temp/duplicate_file_test.jpg";
	File file(FileTest_TestSource);
	EXPECT_TRUE(file.duplicate(duplicateFilePath));

	File duplicateTest(duplicateFilePath);
	EXPECT_TRUE(duplicateTest.isValid());
}

TEST(FileTest, ShouldFailDuplicateAnInvalidFile)
{
	const char* duplicateFilePath = "temp/duplicate_file_test2.jpg";
	File file("somefilethatdoesnotexist.dat");
	EXPECT_FALSE(file.duplicate(duplicateFilePath));

	File duplicateTest(duplicateFilePath);
	EXPECT_FALSE(duplicateTest.isValid());
}

MATCHER_P(IsWithin3Seconds, a, std::string(negation ? "isn't" : "is") +
	" within 3 seconds of " + testing::PrintToString(a)) { return std::abs(int32_t(a)-int32_t(arg)) < 3; }

TEST(FileTest, ShouldReturnLastModificationTimeOfValidFile)
{
	auto timeNow = std::chrono::system_clock::now().time_since_epoch();
	uint64_t secondsSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(timeNow).count();

	const char* filename = "temp/last_modification_file_test.dat";
	writeTestFile<512>(filename);

	File file(filename);
	EXPECT_EQ(secondsSinceEpoch, file.lastModificationTime());
}

TEST(FileTest, ShouldFailReturnLastModificationTimeOfInvalidFile)
{
	File file("someinvalidfilethatdoesnotexist.dat");
	EXPECT_EQ(0, file.lastModificationTime());
}
