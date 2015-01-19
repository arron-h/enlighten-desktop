#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "watcher.h"
#include "ifile.h"
#include <chrono>

using namespace enlighten::lib;

namespace
{
	class MockFile : public IFile
	{
	public:
		MOCK_METHOD0(openRead, bool());
		MOCK_METHOD0(openWrite, bool());

		MOCK_METHOD2(read, uint64_t(uint8_t*, uint32_t));
		MOCK_METHOD2(write, uint64_t(uint8_t*, uint32_t));

		MOCK_METHOD0(close, void());

		MOCK_CONST_METHOD0(filePath, const char*());
		MOCK_METHOD0(fileSize, uint64_t());

		MOCK_METHOD0(isValid, bool());

		MOCK_METHOD1(duplicate, bool(const char*));
		MOCK_METHOD0(remove, bool());
		MOCK_METHOD0(lastModificationTime, uint64_t());
	};

	class MockWatcherDelegate : public AbstractWatcherDelegate
	{
	public:
		MOCK_METHOD2(fileHasChanged, void(Watcher*, const IFile*));

		volatile bool _hasBeenCalled;
	};
}

TEST(WatcherTest, ShouldConstruct)
{
	MockFile file;
	MockWatcherDelegate delegate;

	Watcher watcher(&file, &delegate);
}

TEST(WatcherTest, ShouldWatchValidFileForChanges)
{
	MockFile file;
	MockWatcherDelegate delegate;

	EXPECT_CALL(file, isValid())
		.Times(1)
		.WillRepeatedly(testing::Return(true));

	EXPECT_CALL(file, lastModificationTime())
		.Times(testing::AtLeast(1))
		.WillRepeatedly(testing::Return(0));

	Watcher watcher(&file, &delegate);
	EXPECT_TRUE(watcher.beginWatchingForChanges(1000));

	// Sleep a bit so the worker thread has time to wake up
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST(WatcherTest, ShouldFailWatchNonExistentFileForChanges)
{
	MockFile file;
	MockWatcherDelegate delegate;

	EXPECT_CALL(file, isValid())
		.WillRepeatedly(testing::Return(false));

	Watcher watcher(&file, &delegate);
	EXPECT_FALSE(watcher.beginWatchingForChanges(1000));
}

TEST(WatcherTest, ShouldStopWatchingFileForChanges)
{
	MockFile file;
	MockWatcherDelegate delegate;

	EXPECT_CALL(file, isValid())
		.Times(1)
		.WillRepeatedly(testing::Return(true));

	EXPECT_CALL(file, lastModificationTime())
		.Times(testing::AtLeast(1))
		.WillRepeatedly(testing::Return(0));

	Watcher watcher(&file, &delegate);
	EXPECT_TRUE(watcher.beginWatchingForChanges(1000));

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	EXPECT_TRUE(watcher.isWatching());
	EXPECT_TRUE(watcher.stop());

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	EXPECT_FALSE(watcher.isWatching());
}

TEST(WatcherTest, ShouldWatchFileForChangesMultipleTimes)
{
	MockFile file;
	MockWatcherDelegate delegate;

	EXPECT_CALL(file, isValid())
		.Times(testing::AtLeast(1))
		.WillRepeatedly(testing::Return(true));

	EXPECT_CALL(file, lastModificationTime())
		.Times(testing::AtLeast(1))
		.WillRepeatedly(testing::Return(0));

	Watcher watcher(&file, &delegate);
	EXPECT_TRUE(watcher.beginWatchingForChanges(1000));
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(watcher.stop());
	EXPECT_FALSE(watcher.isWatching());

	EXPECT_TRUE(watcher.beginWatchingForChanges(1000));
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_TRUE(watcher.stop());
	EXPECT_FALSE(watcher.isWatching());
}

TEST(WatcherTest, ShouldCallDelegateWhenWatchedFileChanges)
{
	MockFile file;
	MockWatcherDelegate delegate;
	delegate._hasBeenCalled = false;

	EXPECT_CALL(file, isValid())
		.Times(testing::AtLeast(1))
		.WillRepeatedly(testing::Return(true));

	// Set mock to return a new modification time after 2 polls
	testing::InSequence s;
	EXPECT_CALL(file, lastModificationTime())
		.Times(2)
		.WillRepeatedly(testing::Return(0));
	EXPECT_CALL(file, lastModificationTime())
		.WillRepeatedly(testing::Return(1234));

	Watcher watcher(&file, &delegate);

	// Delegate expectations
	EXPECT_CALL(delegate, fileHasChanged(&watcher, &file))
		.WillOnce(testing::Assign(&delegate._hasBeenCalled, true));

	EXPECT_TRUE(watcher.beginWatchingForChanges(500));

	auto start = std::chrono::system_clock::now();
	while (!delegate._hasBeenCalled)
	{
		std::this_thread::yield();

		auto now = std::chrono::system_clock::now();
		std::chrono::milliseconds elapsedMs =
			std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
		ASSERT_LT(elapsedMs.count(), 5000);	// 5second timeout
	}

	EXPECT_TRUE(watcher.stop());
	EXPECT_FALSE(watcher.isWatching());
}
