#include "logger.h"
#include <cstdarg>
#include <cstdio>

namespace enlighten
{
namespace lib
{
const char* SEVERITY_STRINGS[] =
{
	"Error",   // ERROR
	"Warning", // WARN
	"Info",    // INFO
	"Debug"    // DEBUG
};

Logger::Logger() : _delegate(nullptr)
{
}

Logger& Logger::get()
{
	static Logger loggerInstance;
	return loggerInstance;
}

const char* Logger::stringifySeverity(Severity severity)
{
	return SEVERITY_STRINGS[severity];
}

void Logger::setLoggerDelegate(AbstractLoggerDelegate* delegate)
{
	_delegate = delegate;
}

AbstractLoggerDelegate* Logger::currentDelegate() const
{
	return _delegate;
}

void Logger::log(Severity severity, const char* message, ...)
{
	char logBuffer[256];

	va_list args;
	va_start(args, message);
	vsnprintf(logBuffer, 256, message, args);
	va_end(args);

	if (_delegate)
	{
		_delegate->processLogMessage(severity, logBuffer);
	}
}

} // lib
} // enlighten
