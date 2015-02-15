#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "logger.h"

using namespace enlighten;

class ConsoleLoggerDelegate : public lib::AbstractLoggerDelegate
{
public:
	void processLogMessage(lib::Logger::Severity severity, const char* message)
	{
		printf("  %s | %s\n", lib::Logger::stringifySeverity(severity), message);
	}
};

namespace enlighten { namespace lib {
	const char* applicationSettingsDirectory()
	{
		return "integrationtemp/";
	}
}}

int main(int argc, char **argv)
{
	ConsoleLoggerDelegate delegate;
	lib::Logger::get().setLoggerDelegate(&delegate);

	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();

	lib::Logger::get().setLoggerDelegate(nullptr);

	return result;
}
