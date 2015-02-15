#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "aws/aws.h"
#include "aws/awsrequest.h"
#include "settings.h"
#include "file.h"
#include "cachedpreviews.h"
#include "synchronizers/previewssynchronizer.h"
#include <chrono>

#include "sqlite3.h"

using namespace enlighten::lib;

namespace
{
	static const char* PreviewsSynchronizer_ValidPreviewFile =
		"catalogs/Lightroom 5 Catalog Previews.lrdata/previews.db";

	class MockAwsRequest : public IAwsRequest
	{
	public:
		MOCK_METHOD1(headObject, bool(const std::string& key));
		MOCK_METHOD2(getObject, bool(const std::string& key, AwsGet& get));
		MOCK_METHOD2(putObject, bool(const std::string& key, const AwsPut& put));
		MOCK_METHOD1(removeObject, bool(const std::string& key));

		MOCK_METHOD0(cancel, void());
		MOCK_METHOD0(reset, void());

		MOCK_METHOD0(response, const AwsResponse*());
		MOCK_METHOD0(statusCode, int32_t());
	};

	void removeUuidFromDatabaseTable(const char* database, const char* table, const char* uuid)
	{
		sqlite3* db;
		int dbOpenResult = sqlite3_open_v2(database, &db, SQLITE_OPEN_READWRITE, NULL);

		ASSERT_TRUE(dbOpenResult == SQLITE_OK);

		char queryBuffer[128];
		int written = snprintf(queryBuffer, 128, "DELETE FROM %s WHERE uuid='%s'", table, uuid);
		ASSERT_TRUE(written < 128);

		sqlite3_stmt* statement = nullptr;
		int statementResult = sqlite3_prepare_v2(db, queryBuffer, -1, &statement,
			NULL);
		ASSERT_TRUE(statementResult == SQLITE_OK);

		ASSERT_TRUE(sqlite3_step(statement) == SQLITE_DONE);
	}

	class FakeAws : public IAws
	{
	public:
		FakeAws(MockAwsRequest* mockRequest) : _mockRequest(mockRequest)
		{
		}

		bool initialiseDestinationWithProfile(
			const std::string& destinationIdentifier,
			const AwsAccessProfile& accessProfile,
			const AwsDestination& destination)
		{
			return true;
		}

		IAwsRequest* createRequestForDestination(const std::string& destination)
		{
			return _mockRequest;
		}

		void freeRequest(IAwsRequest*)
		{
		}

	protected:
		IAwsRequest* _mockRequest;
	};

	class PreviewsSynchronizerTest : public testing::Test
	{
	public:
		PreviewsSynchronizerTest() : fakeAws(&mockAwsRequest)
		{
			EXPECT_TRUE(settings.initialise());

			ON_CALL(mockAwsRequest, putObject(testing::_, testing::_))
				.WillByDefault(testing::Return(true));
			ON_CALL(mockAwsRequest, getObject(testing::_, testing::_))
				.WillByDefault(testing::Return(true));
			ON_CALL(mockAwsRequest, headObject(testing::_))
				.WillByDefault(testing::Return(true));
			ON_CALL(mockAwsRequest, removeObject(testing::_))
				.WillByDefault(testing::Return(true));

			ON_CALL(mockAwsRequest, statusCode())
				.WillByDefault(testing::Return(200));
		}
		~PreviewsSynchronizerTest()
		{
			File f(std::string("temp/") + CachedPreviews::databaseFileName());
			f.remove();
		}

	protected:
		Settings settings;
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

	EXPECT_CALL(mockAwsRequest, putObject(testing::_, testing::_))
		.Times(0);
	EXPECT_CALL(mockAwsRequest, removeObject(testing::_))
		.Times(0);
	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile, ""));
}

TEST_F(PreviewsSynchronizerTest, ShouldStartStopSynchronizingFile)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject(testing::_, testing::_))
		.Times(3);
	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile, ""));

	// Wait around awhile
	std::this_thread::sleep_for(std::chrono::seconds(1));

	EXPECT_TRUE(sync.stopSynchronizingFile());
}

TEST_F(PreviewsSynchronizerTest, ShouldFailStopSynchronizingFileIfNotStarted)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_FALSE(sync.stopSynchronizingFile());
}

TEST_F(PreviewsSynchronizerTest, ShouldProcessPreviewsWhenPreviewDatabaseChanges)
{
	settings.set(settings::WatcherPollRate, 100);
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject(testing::_, testing::_))
		.Times(3);
	EXPECT_CALL(mockAwsRequest, removeObject(testing::_))
		.Times(1);

	// Duplicate the database, so we can modify it
	std::string duplicatedDatabaseName = PreviewsSynchronizer_ValidPreviewFile;
	size_t idx = duplicatedDatabaseName.find_last_of(File::pathSeperator());
	duplicatedDatabaseName = duplicatedDatabaseName.substr(0, idx+1) +
		"previewssynchronizer_duplicate.db";
	{
		File file(PreviewsSynchronizer_ValidPreviewFile);
		file.duplicate(duplicatedDatabaseName.c_str());
	}

	EXPECT_TRUE(sync.beginSynchronizingFile(duplicatedDatabaseName, ""));

	// Wait around awhile
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// Inject a change - remove an entry
	removeUuidFromDatabaseTable(duplicatedDatabaseName.c_str(), "ImageCacheEntry",
		"6A2B9912-3868-45E4-AE0D-7EA73F66FF63");
	removeUuidFromDatabaseTable(duplicatedDatabaseName.c_str(), "Pyramid",
		"6A2B9912-3868-45E4-AE0D-7EA73F66FF63");

	// Wait around awhile
	std::this_thread::sleep_for(std::chrono::seconds(1));

	EXPECT_TRUE(sync.stopSynchronizingFile());

	File file(duplicatedDatabaseName);
	EXPECT_TRUE(file.remove());
}

TEST_F(PreviewsSynchronizerTest, ShouldNotProcessPreviewsIfAlreadyProcessing)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_CALL(mockAwsRequest, putObject(testing::_, testing::_))
		.Times(3);

	EXPECT_TRUE(sync.beginSynchronizingFile(PreviewsSynchronizer_ValidPreviewFile, ""));

	// Inject a change
	File file(PreviewsSynchronizer_ValidPreviewFile);
	sync.fileHasChanged(nullptr, &file);

	// Wait around awhile
	std::this_thread::sleep_for(std::chrono::seconds(1));

	EXPECT_TRUE(sync.stopSynchronizingFile());
}

TEST_F(PreviewsSynchronizerTest, WillNotJoinWhenFileChangedDelegateCalledIfNotJoinable)
{
	PreviewsSynchronizer sync(&settings, &fakeAws);

	EXPECT_FALSE(sync.fileHasChanged(nullptr, nullptr));
}
