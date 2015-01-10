#ifndef LOGGER_H
#define LOGGER_H

namespace enlighten
{
namespace lib
{
class AbstractLoggerDelegate;
class Logger
{
public:
	enum Severity
	{
		ERROR,
		WARNING,
		INFO,
		DEBUG
	};

public:
	static Logger& get();
	static const char* stringifySeverity(Severity severity);

	void setLoggerDelegate(AbstractLoggerDelegate* delegate);
	AbstractLoggerDelegate* currentDelegate() const;

	void log(Severity severity, const char* message, ...);
private:
	Logger();

	AbstractLoggerDelegate* _delegate;
};

class AbstractLoggerDelegate
{
public:
	virtual void processLogMessage(Logger::Severity severity, const char* message) = 0;
};

} // lib
} // enlighten

#endif
