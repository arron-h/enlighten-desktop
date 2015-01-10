#include <stdio.h>
#include <stdlib.h>
#include "lrprev.h"
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

int main(int argc, char* argv[])
{
	if (argc <= 2)
		return -1;

	char* filePath = argv[1];
	char* outputPath = argv[2];

	ConsoleLoggerDelegate logger;
	lib::Logger::get().setLoggerDelegate(&logger);

	LrPrev lrPrevFile;
	if (!lrPrevFile.initialiseWithFile(filePath))
		return -1;

	unsigned int  jpegByteCount  = 0;
	unsigned char* jpegBytes = lrPrevFile.extractFromLevel(3, jpegByteCount);

	// Write out the bytes
	FILE* outputFile = fopen(outputPath, "wb");
	fwrite(jpegBytes, 1, jpegByteCount, outputFile);
	fclose(outputFile);

	free(jpegBytes);

	return 0;
};
