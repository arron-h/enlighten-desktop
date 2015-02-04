#ifndef ABSTRACTSYNCHRONIZER_H
#define ABSTRACTSYNCHRONIZER_H

#include <string>

namespace enlighten
{
namespace lib
{
class AWSConfig;
class AbstractSynchronizerProgressDelegate;
class AbstractSynchronizer
{
public:
	AbstractSynchronizer() : _delegate(nullptr) {}
	virtual ~AbstractSynchronizer() {}

	void setProgressDelegate(AbstractSynchronizerProgressDelegate* delegate) { _delegate = delegate; }
	virtual bool beginSynchronizingFile(const std::string& file, const std::string& awsDestinationIdentifier) = 0;
	virtual bool stopSynchronizingFile() = 0;

protected:
	AbstractSynchronizerProgressDelegate* _delegate;
};

class AbstractSynchronizerProgressDelegate
{
public:
	virtual ~AbstractSynchronizerProgressDelegate();
	virtual void synchronizerProgressUpdated(AbstractSynchronizer* synchronizer,
		const std::string& processedText, float percentComplete);
};

} // lib
} // enlighten

#endif
