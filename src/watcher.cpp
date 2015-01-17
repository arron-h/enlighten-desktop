#include "watcher.h"
#include "ifile.h"
#include "validation.h"

#include <chrono>

namespace enlighten
{
namespace lib
{
Watcher::Watcher(IFile* fileToWatch, AbstractWatcherDelegate* delegate) :
	_shouldPoll(false), _file(fileToWatch), _delegate(delegate)
{
}

Watcher::~Watcher()
{
	stop();
}

bool Watcher::beginWatchingForChanges(uint32_t pollRateMs)
{
	VALIDATE(_file->isValid(), "Attempting to watch an invalid file");

	_pollRate = pollRateMs;
	_shouldPoll = true;
	_workerThread = std::thread(&Watcher::pollForChanges, this);

	return true;
}

bool Watcher::stop()
{
	if (isWatching())
	{
		_shouldPoll = false;
		_workerThread.join();
	}

	return true;
}

void Watcher::pollForChanges()
{
	if (!_shouldPoll)
		return;

	uint64_t lastModificationTime = static_cast<uint64_t>(-1);
	while (_shouldPoll)
	{
		uint64_t timeCheck = _file->lastModificationTime();
		if (timeCheck > lastModificationTime)
		{
			_delegate->fileHasChanged(this, _file);
		}
		lastModificationTime = timeCheck;

		// Don't really want to sleep for the entire poll duration as this will cause
		// the app to block for pollRate time when shutting down.
		std::chrono::time_point<std::chrono::system_clock> start, now;
		std::chrono::milliseconds elapsedMs;
		start = std::chrono::system_clock::now();
		do
		{
			if (!_shouldPoll)
				break;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			now = std::chrono::system_clock::now();
			elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
		} while (elapsedMs.count() < _pollRate);
	}

	return;
}

bool Watcher::isWatching() const
{
	return _workerThread.joinable();
}
} // lib
} // enlighten
