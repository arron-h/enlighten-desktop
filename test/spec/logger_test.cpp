#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "logger.h"

using namespace enlighten::lib;

class MockLoggerDelegate : public AbstractLoggerDelegate
{
public:
	MOCK_METHOD2(processLogMessage, void(Logger::Severity severity,
		const char* message));
};

class LoggerTest : public testing::Test
{
public:
	LoggerTest()
	{
		previousLogger = Logger::get().currentDelegate();
	}

	~LoggerTest()
	{
		Logger::get().setLoggerDelegate(previousLogger);
	}

protected:
	AbstractLoggerDelegate* previousLogger;
	MockLoggerDelegate mockDelegate;
};

TEST_F(LoggerTest, ShouldReturnASingletonInstance)
{
	Logger& loggerInstanceA = Logger::get();
	Logger& loggerInstanceB = Logger::get();

	EXPECT_EQ(&loggerInstanceA, &loggerInstanceB);
}

TEST_F(LoggerTest, ShouldBeAbleToStringifySeverityLevels)
{
	Logger::Severity debugSeverity = Logger::DEBUG;

	EXPECT_STREQ("Debug", Logger::stringifySeverity(debugSeverity));
}

TEST_F(LoggerTest, ShouldCallLoggerDelegateWhenADelegateIsSet)
{
	Logger::get().setLoggerDelegate(&mockDelegate);

	EXPECT_CALL(mockDelegate, processLogMessage(Logger::ERROR, testing::_)).Times(1);

	Logger::get().log(Logger::ERROR, "My %s message %d", "log", 1);
}

TEST_F(LoggerTest, ShouldNoopWhenNoDelegateIsSet)
{
	EXPECT_NO_FATAL_FAILURE(Logger::get().log(Logger::INFO, "Some info"));
}
