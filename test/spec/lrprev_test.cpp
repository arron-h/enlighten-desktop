#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "lrprev.h"

TEST(LrPrev, ShouldFailToInitialiseWithAnInvalidFile)
{
	LrPrev lrPrev;
	EXPECT_FALSE(lrPrev.initialiseWithFile("somefile.lrprev"));
	EXPECT_FALSE(lrPrev.initialiseWithFile(NULL));
}
