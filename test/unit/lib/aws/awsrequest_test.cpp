#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rapidjson/document.h"

#include "aws/awsrequest.h"
#include "aws/aws.h"

#include "logger.h"

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

	class AwsRequestTest : public testing::Test
	{
	public:
		AwsRequestTest()
		{
			config.hostName = "https://httpbin.org";
			request = new AwsRequest(&config, &accessProfile, &destination);
		}

		~AwsRequestTest()
		{
			delete request;
		}

		AwsConfig config;
		AwsRequest* request;
	};
}

TEST_F(AwsRequestTest, ShouldConstruct)
{
	// Tested as part of fixture
}

TEST_F(AwsRequestTest, PUT)
{
	uint8_t data[] = { 'H','E','L','L','O' };

	AwsPut put;
	put.data = data;
	put.dataSize = sizeof(data);
	ASSERT_TRUE(request->putObject("put", put));

	const AwsResponse* response = request->response();

	std::string body(reinterpret_cast<char*>(response->body), response->bodySize);
	Logger::get().log(Logger::INFO, "Response body:\n%s", body.c_str());

	rapidjson::Document d;
	d.Parse(body.c_str());

	ASSERT_FALSE(d.HasParseError());
	EXPECT_STREQ("HELLO", d["data"].GetString());
}

TEST_F(AwsRequestTest, DELETE)
{
	ASSERT_TRUE(request->removeObject("delete"));

	const AwsResponse* response = request->response();

	std::string body(reinterpret_cast<char*>(response->body), response->bodySize);
	Logger::get().log(Logger::INFO, "Response body:\n%s", body.c_str());

	rapidjson::Document d;
	d.Parse(body.c_str());

	ASSERT_FALSE(d.HasParseError());
}

TEST_F(AwsRequestTest, HEAD)
{
	ASSERT_TRUE(request->headObject("get"));

	const AwsResponse* response = request->response();

	std::string body(reinterpret_cast<char*>(response->body), response->bodySize);
	Logger::get().log(Logger::INFO, "Response body:\n%s", body.c_str());

	rapidjson::Document d;
	d.Parse(body.c_str());

	ASSERT_FALSE(d.HasParseError());
}

TEST_F(AwsRequestTest, GET)
{
	AwsGet get;
	memset(&get, 0, sizeof(get));
	ASSERT_TRUE(request->getObject("get", get));

	std::string body(reinterpret_cast<char*>(get.buffer), get.bufferSize);
	Logger::get().log(Logger::INFO, "Get body:\n%s", body.c_str());

	rapidjson::Document d;
	d.Parse(body.c_str());

	ASSERT_FALSE(d.HasParseError());

	free(get.buffer);
}
