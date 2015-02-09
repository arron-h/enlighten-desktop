#ifndef PREVIEWSSYNCHRONIZER_H
#define PREVIEWSSYNCHRONIZER_H

#include "abstractsynchronizer.h"
#include "previewentry.h"
#include "watcher.h"
#include "syncaction.h"

#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <map>
#include <string>

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

	bool beginSynchronizingFile(const std::string& file,
		const std::string& awsDestinationIdentifier);
	bool stopSynchronizingFile();

	// WatcherDelegate
	bool fileHasChanged(Watcher* watcher, const IFile* file);

private:
	bool processChanges();
	void crunchAndUpload(std::map<uuid_t, SyncAction>* entries, SuccessCallbackFunc processedUuidCallback,
		 ErrorCallbackFunc processingErrorCallback);
	std::string pathOfPreviewsDatabaseFile();

	void processedUuid(const uuid_t& uuid);
	void errorProcessingUuid(const uuid_t& uuid, const std::string& error);
	void stopAndCleanup();

private:
	PreviewsDatabase* _previewsDatabase;
	CachedPreviews* _cachedPreviews;

	IEnlightenSettings* _settings;
	IAws* _aws;

	std::string _awsDestinationIdentifier;

	Watcher* _watcher;
	IFile* _previewsDatabaseFile;

	std::thread _workerThread;
	std::promise<bool> _workerPromise;
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
