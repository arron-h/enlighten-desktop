#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "aws/aws.h"
#include "aws/awsprofilesloader.h"

using namespace enlighten::lib;

namespace
{
	const std::string profileFile = "awsprofiles.json";

	const AwsAccessProfile accessProfiles[2] =
	{
		{
			std::string("12345"),
			std::string("567890")
		},
		{
			std::string("abcdef"),
			std::string("ghijkl")
		}
	};

	const AwsDestination destinations[3] =
	{
		{
			std::string("my-photos"),
			std::string("nested/key")
		},
		{
			std::string("client-photos"),
			std::string("clients/james")
		},
		{
			std::string("family-photos"),
			std::string("")
		}
	};

	class MockAws : public IAws
	{
	public:
		MOCK_METHOD3(initialiseDestinationWithProfile, bool(const std::string&,
				const AwsAccessProfile&, const AwsDestination&));
		MOCK_METHOD1(createRequestForDestination, IAwsRequest*(const std::string&));
		MOCK_METHOD1(freeRequest, void(IAwsRequest*));
	};

	class AwsProfilesLoaderTest : public testing::Test
	{
	public:
		AwsProfilesLoaderTest()
		{
		}

		AwsProfilesLoader loader;
		MockAws aws;
	};
}

TEST_F(AwsProfilesLoaderTest, ShouldConstruct)
{
	// Tested as part of fixture
}

TEST_F(AwsProfilesLoaderTest, ShouldFailLoadProfileFromInvalidPath)
{
	EXPECT_FALSE(loader.loadProfilesFromPath("invalidfile.json", &aws));
}

TEST_F(AwsProfilesLoaderTest, ShouldInitialiseDestinationOnLoad)
{
	EXPECT_CALL(aws, initialiseDestinationWithProfile(testing::_, testing::_, testing::_))
		.Times(testing::AtLeast(1));
	EXPECT_TRUE(loader.loadProfilesFromPath(profileFile, &aws));
}

TEST_F(AwsProfilesLoaderTest, ShouldInitialiseMultipleDestinationsOnLoad)
{
	EXPECT_CALL(aws, initialiseDestinationWithProfile(
			std::string("ExampleDestination1"), accessProfiles[0], destinations[0]))
		.Times(1);
	EXPECT_CALL(aws, initialiseDestinationWithProfile(
			std::string("ExampleDestination2"), accessProfiles[0], destinations[1]))
		.Times(1);
	EXPECT_CALL(aws, initialiseDestinationWithProfile(
			std::string("ExampleDestination3"), accessProfiles[1], destinations[2]))
		.Times(1);

	EXPECT_TRUE(loader.loadProfilesFromPath(profileFile, &aws));
}
