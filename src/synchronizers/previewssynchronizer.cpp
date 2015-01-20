#include "synchronizers/previewssynchronizer.h"
#include "previewsdatabase.h"
#include "cachedpreviews.h"
#include "jpegcruncher.h"
#include "lrprev.h"
#include "jpeg.h"
#include "file.h"
#include "settings.h"
#include "aws.h"
#include "logger.h"

#include "validation.h"

#include <cstring>
#include <cstdlib>

namespace enlighten
{
namespace lib
{
PreviewsSynchronizer::PreviewsSynchronizer(IEnlightenSettings* settings, IAws* aws) :
	_previewsDatabase(new PreviewsDatabase()),
	_cachedPreviews(new CachedPreviews(settings)),
	_settings(settings), _aws(aws), _watcher(nullptr),
	_previewsDatabaseFile(nullptr), _state(Idle)
{
}

PreviewsSynchronizer::~PreviewsSynchronizer()
{
	if (_state == Synchronizing)
		stopAndCleanup();

	delete _previewsDatabase;
	delete _cachedPreviews;
}

bool PreviewsSynchronizer::beginSynchronizingFile(const std::string& file)
{
	VALIDATE(_state != Synchronizing, "Already in a synchronizing state");

	CHECK(_previewsDatabase->initialiseWithFile(file.c_str()));
	CHECK(_cachedPreviews->loadOrCreateDatabase());

	_previewsDatabaseFile = new File(file);

	int32_t pollRate = _settings->get(IEnlightenSettings::WatcherPollRate, 5000);
	_watcher = new Watcher(_previewsDatabaseFile, this);
	_watcher->beginWatchingForChanges(pollRate);

	Logger::get().log(Logger::INFO, "Starting to synchronize file: %s", file.c_str());

	_state = Synchronizing;

	// Synchronize now
	return processChanges();
}

bool PreviewsSynchronizer::stopSynchronizingFile()
{
	VALIDATE(_state == Synchronizing, "Not in a synchronizing state");

	stopAndCleanup();
	_state = Idle;

	return true;
}

void PreviewsSynchronizer::stopAndCleanup()
{
	if (_workerThread.joinable())
	{
		_cancelWorking = true;
		_workerThread.join();
	}

	if (_watcher)
	{
		_watcher->stop();

		delete _watcher;
		_watcher = nullptr;
	}

	if (_previewsDatabaseFile)
	{
		delete _previewsDatabaseFile;
		_previewsDatabaseFile = nullptr;
	}
}

void PreviewsSynchronizer::fileHasChanged(Watcher* watcher, const IFile* file)
{
	if (_workerThread.joinable()) // Skip processing if we're currently working
		return;

	processChanges();
}

bool PreviewsSynchronizer::processChanges()
{
	Logger::get().log(Logger::INFO, "Processing changes");

	CHECK(_previewsDatabase->reopen());

	std::map<uuid_t, SyncAction>* uuidActions = new std::map<uuid_t, SyncAction>;
	CHECK(_previewsDatabase->checkEntriesAgainstCachedPreviews(*_cachedPreviews, *uuidActions));

	if (uuidActions->size() > 0)
	{
		// This little lovely allows us to call this->processedUuid from the worker thread.
		auto uuidProcessCallback = std::bind(&PreviewsSynchronizer::processedUuid,
			this, std::placeholders::_1);
		auto errorProcessingCallback = std::bind(&PreviewsSynchronizer::errorProcessingUuid,
			this, std::placeholders::_1, std::placeholders::_2);

		Logger::get().log(Logger::INFO, "%u changes found. Processing...", uuidActions->size());

		// Kick off the worker thread
		_cancelWorking = false;
		_workerThread  = std::thread(&PreviewsSynchronizer::crunchAndUpload, this,
			uuidActions, uuidProcessCallback, errorProcessingCallback);
	}

	return true;
}

void PreviewsSynchronizer::crunchAndUpload(std::map<uuid_t, SyncAction>* entries, SuccessCallbackFunc processedUuidCallback,
		 ErrorCallbackFunc processingErrorCallback)
{
	if (!entries)
	{
		return;
	}

	auto it = entries->begin();
	for (; it != entries->end() && !_cancelWorking; it++)
	{
		Logger::get().log(Logger::INFO, "Crunching uuid %s", it->first.c_str());

		// load it
		const PreviewEntry* entry;
		{
			std::lock_guard<std::mutex> autolock(_mutex);
			entry =  _previewsDatabase->entryForUuid(it->first);
		}

		if (!entry)
		{
			processingErrorCallback(it->first, "Failed to find entry '"+ it->first +"' in database");
			continue;
		}

		LrPrev prev;
		const std::string& filePath = entry->filePathRelativeToRoot();
		if (!prev.initialiseWithFile(filePath.c_str()))
		{
			processingErrorCallback(it->first, "Failed to load LrPrev for entry '"+ it->first +"'");
			continue;
		}

		int32_t previewLongestDimension = _settings->get(IEnlightenSettings::PreviewLongestDimension, 220);
		int32_t previewQuality          = _settings->get(IEnlightenSettings::PreviewQuality, 40);

		uint32_t desiredLevel = entry->closestLevelToDimension(static_cast<float>(previewLongestDimension));
		if (desiredLevel == PreviewEntry::INVALID_LEVEL_INDEX)
		{
			processingErrorCallback(it->first, "No appropriate of levels exist for entry '"+ it->first +"'");
			continue;
		}

		uint32_t jpegSize;
		uint8_t* jpegData = prev.extractFromLevel(desiredLevel, jpegSize);
		if (!jpegData)
		{
			processingErrorCallback(it->first, "Failed to extract Jpeg data for entry '"+ it->first +"'");
			continue;
		}

		Jpeg sourceJpeg(jpegData, jpegSize, false);
		Jpeg targetJpeg;

		// Crunch it
		if (_cancelWorking)
		{
			break;
		}

		JpegCruncher cruncher(&sourceJpeg, &targetJpeg);
		bool crunched = cruncher.reencodeJpeg(previewLongestDimension, previewQuality);
		free(jpegData);

		if (!crunched)
		{
			processingErrorCallback(it->first, "Failed to reencode Jpeg data for entry '"+ it->first +"'");
			continue;
		}

		// Upload it
		if (_cancelWorking)
		{
			break;
		}

		// TODO - in progress!
		IAwsRequest* request = _aws->createRequestForBucket("");
		SyncAction action = it->second;
		switch(action)
		{
			case SyncAction_Add:
				request->putObject();
			break;
			case SyncAction_Remove:
				request->removeObject();
			break;
			case SyncAction_Update:
			break;
		}
		delete request;

		// Notify done
		processedUuidCallback(it->first);
	}

	// Delete the memory holding the entries
	delete entries;
}

void PreviewsSynchronizer::processedUuid(const uuid_t& uuid)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	_cachedPreviews->markAsCached(uuid);
}

void PreviewsSynchronizer::errorProcessingUuid(const uuid_t& uuid, const std::string& error)
{
	std::lock_guard<std::mutex> autolock(_mutex);
	Logger::get().log(Logger::DEBUG, "%s", error.c_str());
}
} // lib
} // enlighten
