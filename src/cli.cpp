#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "settings.h"
#include "aws/aws.h"
#include "logger.h"
#include "paths.h"
#include "scanner.h"
#include "synchronizers/previewssynchronizer.h"

#include <thread>
#include <csignal>

using namespace enlighten;

namespace
{
	volatile sig_atomic_t applicationSignalStatus = 0;
	lib::Paths platformPaths;

	class ConsoleLoggerDelegate : public lib::AbstractLoggerDelegate
	{
	public:
		void processLogMessage(lib::Logger::Severity severity, const char* message)
		{
			printf("  %s | %s\n", lib::Logger::stringifySeverity(severity), message);
		}
	};

	void applicationSignalHandler(int signal)
	{
		applicationSignalStatus = signal;
	}
}

namespace enlighten { namespace lib {
	const char* applicationSettingsDirectory()
	{
		return platformPaths.applicationSettings();
	}
}}

void printUsage(const char* arg0)
{
	puts("Enlighten Desktop | Development CLI");
	puts("Usage:");
	printf("\t%s <path/to/lightroom_files/> <aws_destination_identifier>\n", arg0);
}

int main(int argc, char* argv[])
{
	if (argc <= 2)
	{
		printUsage(argv[0]);
		return -1;
	}

	signal(SIGINT, applicationSignalHandler);
	signal(SIGTERM, applicationSignalHandler);

	ConsoleLoggerDelegate logger;
	lib::Logger::get().setLoggerDelegate(&logger);

	char* filePath = argv[1];
	char* awsDestination = argv[2];

	lib::Settings settings;
	lib::Aws& aws = lib::Aws::get();

	lib::Scanner scanner;
	if (!scanner.scanForLightroomFilesAtPath(filePath))
	{
		lib::Logger::get().log(lib::Logger::ERROR, "Failed to find a valid Lightroom file pair");
		return 1;
	}

	std::vector<lib::AbstractSynchronizer*> synchronizers;

	const lib::Scanner::LightroomFilePairs& filePairs = scanner.lightroomFilePairs();
	for (auto it = filePairs.begin(); it != filePairs.end(); it++)
	{
		const std::string& previews = it->previews;

		lib::Logger::get().log(lib::Logger::INFO, "Synchronizing '%s'", previews.c_str());

		lib::PreviewsSynchronizer* previewsSync = new lib::PreviewsSynchronizer(&settings, &aws);
		if (previewsSync->beginSynchronizingFile(previews, awsDestination))
		{
			synchronizers.push_back(previewsSync);
		}
		else
		{
			delete previewsSync;
		}
	}

	while (synchronizers.size() > 0 && applicationSignalStatus == 0)
	{
		std::this_thread::yield();
	}

	lib::Logger::get().log(lib::Logger::INFO, "Cleaning up %d synchronizers", synchronizers.size());

	// Cleanup
	for (int syncIdx = 0; syncIdx < synchronizers.size(); ++syncIdx)
	{
		delete synchronizers[syncIdx];
		synchronizers[syncIdx] = nullptr;
	}

	return 0;
};
