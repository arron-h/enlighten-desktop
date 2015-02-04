#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "scanner.h"
#include "ifile.h"

using namespace enlighten::lib;

namespace
{
}

TEST(ScannerTest, ShouldConstruct)
{
	Scanner scanner;
}

TEST(ScannerTest, ShouldFindFilePairAtValidPath)
{
	Scanner scanner;
	EXPECT_TRUE(scanner.scanForLightroomFilesAtPath("catalogs/"));
}

TEST(ScannerTest, ShouldFailFindPairAtInvalidPath)
{
	Scanner scanner;
	EXPECT_FALSE(scanner.scanForLightroomFilesAtPath("an/invalid/path/"));
}

TEST(ScannerTest, ShouldFailFindPairsAtValidPath)
{
	Scanner scanner;
	EXPECT_FALSE(scanner.scanForLightroomFilesAtPath("./"));
}

TEST(ScannerTest, ShouldRetrieveFilePairs)
{
	Scanner scanner;
	EXPECT_TRUE(scanner.scanForLightroomFilesAtPath("catalogs/"));

	const Scanner::LightroomFilePairs& pairs = scanner.lightroomFilePairs();
	EXPECT_GE(pairs.size(), 1);

	EXPECT_FALSE(pairs.begin()->catalog.length() == 0);
	EXPECT_FALSE(pairs.begin()->previews.length() == 0);

	// TODO Do a regex match for extension
	//EXPECT_EQ(pairs.begin()->catalog, );
	//EXPECT_EQ(pairs.begin()->previews, );
}
