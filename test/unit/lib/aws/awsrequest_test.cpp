#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "aws/awsrequest.h"
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
}

TEST(AwsRequestTest, ShouldConstruct)
{
	AwsRequest request(&accessProfile, &destination);
}


