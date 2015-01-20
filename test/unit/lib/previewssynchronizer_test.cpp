#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "aws.h"
#include "awsrequest.h"
#include "settings.h"
#include "file.h"
#include "synchronizers/previewssynchronizer.h"
#include <chrono>

using namespace enlighten::lib;

namespace
{
	static const char* PreviewsSynchronizer_ValidPreviewFile =
		"catalogs/Lightroom 5 Catalog Previews.lrdata/previews.db";

	class MockAwsRequest : public IAwsRequest
	{
	public:
		MOCK_METHOD0(headObject, bool());
		MOCK_METHOD0(getObject, bool());
		MOCK_METHOD0(putObject, bool());
		MOCK_METHOD0(removeObject, bool());
	};

	class FakeAws : public IAws
	{
	public:
		FakeAws(MockAwsRequest* mockRequest) : _mockRequest(mockRequest)
		{
		}

		IAwsRequest* createRequestForBucket(const std::string& bucket)
		{
			return _mockRequest;
		}

	protected:
		IAwsRequest* _mockRequest;
	};

	class PreviewsSynchronizerTest : public testing::Test
	{
	public:
		PreviewsSynchronizerTest() : fakeAws(&mockAwsRequest)
		{
		}

	protected:
		EnlightenSettings settings;
		MockAwsRequest mockAwsRequest;
		FakeAws fakeAws;
	};
}

TEST_F(PreviewsSynchronizerTest, ShouldConstructWithSettings)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);
}

TEST_F(PreviewsSynchronizerTest, ShouldStartAndCancelSynchronizingFile)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject())
		.Times(0);
	EXPECT_CALL(mockAwsRequest, removeObject())
		.Times(0);
	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile));
}

TEST_F(PreviewsSynchronizerTest, ShouldStartStopSynchronizingFile)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject())
		.Times(3);
	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile));

	// Wait around awhile
	std::this_thread::sleep_for(std::chrono::seconds(3));

	EXPECT_TRUE(sync.stopSynchronizingFile());
}

TEST_F(PreviewsSynchronizerTest, ShouldFailStopSynchronizingFileIfNotStarted)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_FALSE(sync.stopSynchronizingFile());
}

TEST_F(PreviewsSynchronizerTest, ShouldProcessPreviewsWhenPreviewDatabaseChanges)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject())
		.Times(3);
	EXPECT_CALL(mockAwsRequest, removeObject())
		.Times(1);

	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile));

	// Wait around awhile
	std::this_thread::sleep_for(std::chrono::seconds(3));

	// Inject a change - remove an entry
	File file(PreviewsSynchronizer_ValidPreviewFile);
	sync.fileHasChanged(nullptr, &file);

	EXPECT_TRUE(sync.stopSynchronizingFile());
}

TEST_F(PreviewsSynchronizerTest, ShouldNotProcessPreviewsIfAlreadyProcessing)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject())
		.Times(3);

	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile));

	// Inject a change
	File file(PreviewsSynchronizer_ValidPreviewFile);
	sync.fileHasChanged(nullptr, &file);

	EXPECT_TRUE(sync.stopSynchronizingFile());
}
