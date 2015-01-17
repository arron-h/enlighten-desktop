#ifndef WATCHER_H
#define WATCHER_H

#include <string>
#include <cstdint>
#include <thread>

namespace enlighten
{
namespace lib
{
class Watcher;
class IFile;
class AbstractWatcherDelegate
{
public:
	virtual ~AbstractWatcherDelegate() {}

	virtual void fileHasChanged(Watcher* watcher, const IFile* file) = 0;
};

class Watcher
{
public:
	Watcher(IFile* fileToWatch, AbstractWatcherDelegate* delegate);
	~Watcher();

	bool beginWatchingForChanges(uint32_t pollRateMs);
	bool stop();

	bool isWatching() const;

private:
	void pollForChanges();

	std::thread _workerThread;
	volatile bool _shouldPoll;

	IFile* _file;
	AbstractWatcherDelegate* _delegate;
	uint32_t _pollRate;
};
} // lib
} // enlighten

#endif
