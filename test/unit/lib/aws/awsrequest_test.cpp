#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rapidjson/document.h"

#include "aws/awsrequest.h"
#include "aws/aws.h"

#include "logger.h"

#include <thread>
#include <chrono>

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

	uint8_t rawBytes[] = { 'H','E','L','L','O' };

	class AwsRequestTest : public testing::Test
	{
	public:
		AwsRequestTest()
		{
			config.hostName = "https://httpbin.org";
			request = new AwsRequest(&config, &accessProfile, &destination);

			put.data = rawBytes;
			put.dataSize = sizeof(rawBytes);

			memset(&get, 0, sizeof(get));

			EXPECT_EQ(AwsRequest::StateIdle, request->state());
			EXPECT_TRUE(request->response() == nullptr);
		}

		~AwsRequestTest()
		{
			if (get.buffer)
			{
				free(get.buffer);
			}
			delete request;
		}

		AwsConfig config;
		AwsRequest* request;

		AwsPut put;
		AwsGet get;
	};
}

TEST_F(AwsRequestTest, ShouldConstruct)
{
	// Tested as part of fixture
}

TEST_F(AwsRequestTest, PUT)
{
	EXPECT_TRUE(request->putObject("put", put));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
	EXPECT_FALSE(request->response() == nullptr);
}

TEST_F(AwsRequestTest, PUTShouldHandleMultipleRequests)
{
	EXPECT_TRUE(request->putObject("put", put));

	request->reset();
	EXPECT_EQ(AwsRequest::StateIdle, request->state());

	EXPECT_TRUE(request->putObject("put", put));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
	EXPECT_FALSE(request->response() == nullptr);
}

TEST_F(AwsRequestTest, PUTShouldFailIfNotReset)
{
	EXPECT_TRUE(request->putObject("put", put));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());

	EXPECT_FALSE(request->putObject("put", put));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
}

TEST_F(AwsRequestTest, PUTShouldAsyncCancel)
{
	AwsRequest* req = request;
	auto threadFunc = [req](uint32_t timeout)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
		req->cancel();
	};

	// Run a few iterations
	for (int i=0; i < 10; ++i)
	{
		req->reset();
		uint32_t timeout = rand() % 100;
		auto threadHandle = std::thread(threadFunc, timeout);

		EXPECT_FALSE(req->putObject("put", put));
		EXPECT_EQ(AwsRequest::StateCancelled, req->state());
		EXPECT_TRUE(req->response() == nullptr);

		threadHandle.join();
	}
}

TEST_F(AwsRequestTest, PUTShouldReturnValidResponse)
{
	ASSERT_TRUE(request->putObject("put", put));

	const AwsResponse* response = request->response();

	std::string body(reinterpret_cast<char*>(response->body), response->bodySize);
	Logger::get().log(Logger::INFO, "Response body:\n%s", body.c_str());

	rapidjson::Document d;
	d.Parse(body.c_str());

	ASSERT_FALSE(d.HasParseError());
	EXPECT_STREQ("HELLO", d["data"].GetString());
}

TEST_F(AwsRequestTest, PUTShouldHaveValidStatusCode)
{
	ASSERT_TRUE(request->putObject("put", put));
	EXPECT_EQ(200, request->statusCode());
}

TEST_F(AwsRequestTest, DELETE)
{
	// AWS DELETE always returns statusCode 204, so we need to use that API on httpbin
	EXPECT_TRUE(request->removeObject("status/204"));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
	EXPECT_EQ(204, request->statusCode());
}

TEST_F(AwsRequestTest, DELETEShouldHandleMultipleRequests)
{
	EXPECT_TRUE(request->removeObject("status/204"));

	request->reset();
	EXPECT_EQ(AwsRequest::StateIdle, request->state());

	EXPECT_TRUE(request->removeObject("status/204"));
}

TEST_F(AwsRequestTest, DELETEShouldFailIfNotReset)
{
	EXPECT_TRUE(request->removeObject("status/204"));
	EXPECT_FALSE(request->removeObject("status/204"));
}

TEST_F(AwsRequestTest, DELETEShouldFailIfStatusIsNotNoContent)
{
	EXPECT_FALSE(request->removeObject("status/200"));
}

TEST_F(AwsRequestTest, HEAD)
{
	ASSERT_TRUE(request->headObject("get"));
	EXPECT_EQ(200, request->statusCode());
	EXPECT_FALSE(request->response() == nullptr);
}

TEST_F(AwsRequestTest, HEADShouldHandleMultipleRequests)
{
	EXPECT_TRUE(request->headObject("get"));

	request->reset();
	EXPECT_EQ(AwsRequest::StateIdle, request->state());

	EXPECT_TRUE(request->headObject("get"));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
	EXPECT_FALSE(request->response() == nullptr);
}

TEST_F(AwsRequestTest, HEADShouldFailIfNotReset)
{
	EXPECT_TRUE(request->headObject("get"));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());

	EXPECT_FALSE(request->headObject("get"));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
}

TEST_F(AwsRequestTest, HEADShouldHaveETagInResponseHeaders)
{
	ASSERT_TRUE(request->headObject("response-headers?ETag=\"1234567890ABCDEF\""));

	const AwsResponse* response = request->response();
	EXPECT_EQ(response->eTag, "1234567890ABCDEF");
}

TEST_F(AwsRequestTest, GET)
{
	ASSERT_TRUE(request->getObject("bytes/512", get));
	ASSERT_EQ(AwsRequest::StateComplete, request->state());

	const AwsResponse* response = request->response();
	ASSERT_TRUE(response != nullptr);

	EXPECT_EQ(512ull, response->contentLength);
	EXPECT_EQ("application/octet-stream", response->contentType);

	EXPECT_EQ(512ull, get.bufferSize);
	EXPECT_TRUE(get.buffer != nullptr);
}

TEST_F(AwsRequestTest, GETShouldHandleMultipleRequests)
{
	EXPECT_TRUE(request->getObject("bytes/512", get));

	free(get.buffer);
	memset(&get, 0, sizeof(get));

	request->reset();
	EXPECT_EQ(AwsRequest::StateIdle, request->state());

	EXPECT_TRUE(request->getObject("bytes/512", get));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
	EXPECT_FALSE(request->response() == nullptr);
}

TEST_F(AwsRequestTest, GETShouldFailIfNotReset)
{
	EXPECT_TRUE(request->getObject("bytes/512", get));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());

	EXPECT_FALSE(request->getObject("bytes/512", get));
	EXPECT_EQ(AwsRequest::StateComplete, request->state());
}

TEST_F(AwsRequestTest, GETShouldAsyncCancel)
{
	AwsRequest* req = request;
	auto threadFunc = [req](uint32_t timeout)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
		req->cancel();
	};

	// Run a few iterations
	for (int i=0; i < 10; ++i)
	{
		req->reset();
		uint32_t timeout = rand() % 100;
		auto threadHandle = std::thread(threadFunc, timeout);

		EXPECT_FALSE(req->getObject("/bytes/1024", get));
		EXPECT_EQ(AwsRequest::StateCancelled, req->state());

		EXPECT_TRUE(req->response() == nullptr);
		EXPECT_TRUE(get.buffer == nullptr);
		EXPECT_EQ(0ull, get.bufferSize);

		threadHandle.join();
	}
}

TEST_F(AwsRequestTest, GETShouldHaveValidStatusCode)
{
	ASSERT_TRUE(request->getObject("bytes/256", get));
	EXPECT_EQ(200, request->statusCode());
}

TEST_F(AwsRequestTest, ShouldFailWhenCredentialsAreInvalid)
{
	{
		AwsRequest badRequest(&config, nullptr, &destination);
		EXPECT_FALSE(badRequest.headObject("get"));
	}
	{
		AwsRequest badRequest(&config, &accessProfile, nullptr);
		EXPECT_FALSE(badRequest.headObject("get"));
	}
}

TEST_F(AwsRequestTest, ShouldFailWhenHostnameIsInvalid)
{
	AwsConfig cfg;
	cfg.hostName = "http://thisurldoesnotexistihopelol.hello";
	AwsRequest badRequest(&cfg, &accessProfile, &destination);
	EXPECT_FALSE(badRequest.headObject("get"));
}

TEST_F(AwsRequestTest, ShouldFailWhenStatusCodeIsNotExpected)
{
	EXPECT_FALSE(request->putObject("status/403", put));
	EXPECT_EQ(403, request->statusCode());
	request->reset();

	EXPECT_FALSE(request->getObject("status/403", get));
	EXPECT_EQ(403, request->statusCode());
	request->reset();

	EXPECT_FALSE(request->headObject("status/403"));
	EXPECT_EQ(403, request->statusCode());
	request->reset();

	EXPECT_FALSE(request->removeObject("status/403"));
	EXPECT_EQ(403, request->statusCode());
	request->reset();
}

TEST_F(AwsRequestTest, ShouldReturnNullResponseIfNotComplete)
{
	EXPECT_NE(AwsRequest::StateComplete, request->state());
	EXPECT_TRUE(nullptr == request->response());
}
