#ifndef PREVIEWSSYNCHRONIZER_H
#define PREVIEWSSYNCHRONIZER_H

#include "abstractsynchronizer.h"
#include "previewentry.h"
#include "watcher.h"
#include "syncaction.h"

#include <thread>
#include <mutex>
#include <functional>
#include <map>

namespace enlighten
{
namespace lib
{
class PreviewsDatabase;
class CachedPreviews;
class Watcher;
class IEnlightenSettings;
class IFile;
class IAws;
class PreviewsSynchronizer : public AbstractSynchronizer, public AbstractWatcherDelegate
{
private:
	typedef std::function<void(const uuid_t&)> SuccessCallbackFunc;
	typedef std::function<void(const uuid_t&, const std::string&)> ErrorCallbackFunc;

public:
	PreviewsSynchronizer(IEnlightenSettings* settings, IAws* aws);
	~PreviewsSynchronizer();

	bool beginSynchronizingFile(const std::string& file);
	bool stopSynchronizingFile();

	// WatcherDelegate
	void fileHasChanged(Watcher* watcher, const IFile* file);

private:
	bool processChanges();
	void crunchAndUpload(std::map<uuid_t, SyncAction>* entries, SuccessCallbackFunc processedUuidCallback,
		 ErrorCallbackFunc processingErrorCallback);

	void processedUuid(const uuid_t& uuid);
	void errorProcessingUuid(const uuid_t& uuid, const std::string& error);
	void stopAndCleanup();

private:
	PreviewsDatabase* _previewsDatabase;
	CachedPreviews* _cachedPreviews;

	IEnlightenSettings* _settings;
	IAws* _aws;

	Watcher* _watcher;
	IFile* _previewsDatabaseFile;

	std::thread _workerThread;
	volatile bool _cancelWorking;
	std::mutex _mutex;
	
	enum State
	{
		Idle,
		Synchronizing,
	} _state;
};
} // lib
} // enlighten

#endif
