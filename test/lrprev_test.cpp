#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "lrprev.h"

TEST(LrPrev, ShouldFailToInitialiseWithAnInvalidFile)
{
	LrPrev lrPrev;
	ASSERT_EQ(lrPrev.initialiseWithFile("somefile.lrprev"), false);
	ASSERT_EQ(lrPrev.initialiseWithFile(NULL), false);
}
