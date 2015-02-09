#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "aws/aws.h"

using namespace enlighten::lib;

namespace
{
	AwsAccessProfile accessProfile = {
		"123456", // accessKeyId
		"7890"    // secretAccessKey
	};

	AwsDestination destination = {
		"my-bucket", // bucket
		"some/key"   // key
	};

	class AwsTest : public testing::Test
	{
	public:
		AwsTest() : aws(Aws::get())
		{
		}

		~AwsTest()
		{
			aws.removeAllDestinations();
		}

		Aws& aws;
	};
}

TEST_F(AwsTest, ShouldBeSingleton)
{
	Aws& instanceA = Aws::get();
	Aws& instanceB = Aws::get();

	EXPECT_EQ(&instanceA, &instanceB);
}

TEST_F(AwsTest, ShouldInitialiseDestinationWithCredentials)
{
	EXPECT_TRUE(aws.initialiseDestinationWithProfile(
		"MyProfile", accessProfile, destination));
}

TEST_F(AwsTest, ShouldFailInitialiseDuplicateDestinations)
{
	EXPECT_TRUE(aws.initialiseDestinationWithProfile(
		"MyProfile", accessProfile, destination));
	EXPECT_FALSE(aws.initialiseDestinationWithProfile(
		"MyProfile", accessProfile, destination));
}

TEST_F(AwsTest, ShouldSanityCheckAccessProfile)
{
	AwsAccessProfile brokenProfileA = { "12345", "" };
	AwsAccessProfile brokenProfileB = { "", "56789" };

	EXPECT_FALSE(aws.initialiseDestinationWithProfile(
		"MyProfile", brokenProfileA, destination));
	EXPECT_FALSE(aws.initialiseDestinationWithProfile(
		"MyProfile", brokenProfileB, destination));
}

TEST_F(AwsTest, ShouldSanityCheckDestination)
{
	AwsDestination brokenDestination = { "", "" };

	EXPECT_FALSE(aws.initialiseDestinationWithProfile(
		"MyProfile", accessProfile, brokenDestination));
}

TEST_F(AwsTest, ShouldCreateRequestForValidDestination)
{
	EXPECT_TRUE(aws.initialiseDestinationWithProfile(
		"MyProfile", accessProfile, destination));

	IAwsRequest* request = aws.createRequestForDestination("MyProfile");

	EXPECT_TRUE(request != nullptr);

	aws.freeRequest(request);
}

TEST_F(AwsTest, ShouldFailCreateRequestForUninitialisedDestination)
{
	IAwsRequest* request = aws.createRequestForDestination("InvalidProfile");
	EXPECT_TRUE(request == nullptr);
}

