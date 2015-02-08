#include "aws/awsrequest.h"
#include "aws/aws.h"
#include "validation.h"
#include "logger.h"

#include <curl/curl.h>
#include <regex>
#include <cstdlib>
#include <cassert>

namespace enlighten
{
namespace lib
{
class AwsRequestPrivate
{
public:
	AwsRequestPrivate() : curlHandle(nullptr)
	{
	}

	~AwsRequestPrivate()
	{
		curl_easy_cleanup(curlHandle);
		curlHandle = nullptr;
	}

	void createHandle()
	{
		curlHandle = curl_easy_init();
	}

	void prepareHandle()
	{
		curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1);
	}

	void reset()
	{
		curl_easy_reset(curlHandle);
	}

	CURL* curlHandle;
};

AwsRequest::AwsRequest(const AwsConfig* config, const AwsAccessProfile* accessProfile,
		const AwsDestination* destination) :
	_config(config), _accessProfile(accessProfile), _destination(destination),
	_privateImpl(new AwsRequestPrivate)
{
	reset();
}

AwsRequest::~AwsRequest()
{
	reset();
	delete _privateImpl;
}

bool AwsRequest::headObject(const std::string& key)
{
	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "HEAD %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare HEAD request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	curl_easy_setopt(handle, CURLOPT_NOBODY, 1l);
	curl_easy_setopt(handle, CURLOPT_HTTPGET, 1l);
	CURLcode performResult = curl_easy_perform(handle);
	curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &_statusCode);

	if (_cancel)
	{
		_state = StateCancelled;
		return false;
	}

	if(performResult != CURLE_OK)
	{
		Logger::get().log(Logger::ERROR, "curl_easy_perform() failed: %s",
			curl_easy_strerror(performResult));
	}

	_state = StateComplete;

	return performResult == CURLE_OK;
}

bool AwsRequest::getObject(const std::string& key, AwsGet& get)
{
	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "GET %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare GET request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	assert(get.buffer == nullptr);
	assert(get.bufferSize == 0);

	_receivedData = 0l;
	_currentWriteData = &get;

	curl_easy_setopt(handle, CURLOPT_HTTPGET, 1l);
	CURLcode performResult = curl_easy_perform(handle);
	curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &_statusCode);

	_currentWriteData = nullptr;

	if (_cancel)
	{
		_state = StateCancelled;
		return false;
	}

	if(performResult != CURLE_OK)
	{
		Logger::get().log(Logger::ERROR, "curl_easy_perform() failed: %s",
			curl_easy_strerror(performResult));
	}

	_state = StateComplete;

	return performResult == CURLE_OK;
}

bool AwsRequest::putObject(const std::string& key, const AwsPut& put)
{
	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "PUT %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare PUT request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	_transferredData = 0l;
	_currentReadData = &put;

	curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/octet-stream");	// Only support binary files atm

	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(handle, CURLOPT_UPLOAD, 1l);
	curl_easy_setopt(handle, CURLOPT_PUT, 1l);
	curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,
		static_cast<curl_off_t>(put.dataSize));

	CURLcode performResult = curl_easy_perform(handle);
	curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &_statusCode);

	_currentReadData = nullptr;

	if (_cancel)
	{
		_state = StateCancelled;
		return false;
	}

	if(performResult != CURLE_OK)
	{
		Logger::get().log(Logger::ERROR, "curl_easy_perform() failed: %s",
			curl_easy_strerror(performResult));
	}

	_state = StateComplete;

	return performResult == CURLE_OK;
}

bool AwsRequest::removeObject(const std::string& key)
{
	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "DELETE %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare DELETE request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
	CURLcode performResult = curl_easy_perform(handle);
	curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &_statusCode);

	if (_cancel)
	{
		_state = StateCancelled;
		return false;
	}

	if(performResult != CURLE_OK)
	{
		Logger::get().log(Logger::ERROR, "curl_easy_perform() failed: %s",
			curl_easy_strerror(performResult));
	}

	_state = StateComplete;

	return performResult == CURLE_OK;
}

void AwsRequest::cancel()
{
	_cancel = true;
}

void AwsRequest::reset()
{
	_state = StateIdle;
	_statusCode = 0;
	_cancel = false;

	_response.contentLength = 0;
	_response.eTag.clear();
	_response.contentType.clear();
	_response.bodySize = 0;
	if (_response.body)
	{
		free(_response.body);
		_response.body = nullptr;
	}

	_currentWriteData = nullptr;
	_currentReadData = nullptr;
	_receivedData = 0l;
	_transferredData = 0l;
}

const AwsResponse* AwsRequest::response()
{
	if (_state == StateComplete)
		return &_response;

	return nullptr;
}

int32_t AwsRequest::statusCode()
{
	return _statusCode;
}

std::string AwsRequest::prepareUrl(const std::string& key)
{
	std::string url = _config->hostName;
	if (!key.empty())
	{
		url += "/" + key;
	}
	return url;
}

bool AwsRequest::prepareRequest(const std::string& url)
{
	if (!_privateImpl->curlHandle)
	{
		_privateImpl->createHandle();
	}

	if (_state != StateIdle)
	{
		reset();
		_privateImpl->reset();
		_privateImpl->prepareHandle();
	}

	CURL* curlHandle = _privateImpl->curlHandle;
	_state = StateOpening;

	curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_READFUNCTION, readCallback);
	curl_easy_setopt(curlHandle, CURLOPT_READDATA, this);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA , this);
	curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, headerCallback);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEHEADER, this);

	return true;
}

int32_t AwsRequest::onRequest(char *buffer, size_t size, size_t nitems)
{
	if (_cancel)
	{
		return CURL_READFUNC_ABORT;
	}

	if (_state != StateTransferring)
	{
		_state = StateTransferring;
	}

	uint32_t bufferSize = 0;
	if (_currentReadData)
	{
		bufferSize = std::min(size*nitems,
			static_cast<size_t>(_currentReadData->dataSize));

		memcpy(buffer + _transferredData, _currentReadData->data, bufferSize);
		_transferredData += bufferSize;
	}

	return bufferSize;
}

int32_t AwsRequest::onResponse(char *ptr, size_t size, size_t nmemb)
{
	if (_cancel)
	{
		return 0;
	}

	if (_state != StateReceiving)
	{
		_state = StateReceiving;
	}

	uint32_t receivedBufferSize = size * nmemb;

	auto writeToBuffer = [this, receivedBufferSize, ptr](uint8_t** buffer, uint64_t& bufferSize)
	{
		*buffer = (uint8_t*)realloc(*buffer, _receivedData + receivedBufferSize);
		memcpy(*buffer + _receivedData, ptr, receivedBufferSize);
		bufferSize = _receivedData + receivedBufferSize;
	};

	if (_currentWriteData)
	{
		writeToBuffer(&_currentWriteData->buffer, _currentWriteData->bufferSize);
	}
	else
	{
		writeToBuffer(&_response.body, _response.bodySize);
	}

	_receivedData += receivedBufferSize;

	return receivedBufferSize;
}

int32_t AwsRequest::onHeader(char *ptr, size_t size, size_t nmemb)
{
	if (_cancel)
	{
		return 0;
	}

	std::string headerContent(ptr, size * nmemb);

	std::regex contentLengthRx("Content-Length: (\\d+)\r\n");
	std::regex contentTypeRx("Content-Type: (.+)\r\n");
	std::regex eTagTypeRx("ETag: \\\"(.+)\\\"\r\n");

	// Try match content-length
	std::smatch matches;
	{
		std::regex_match(headerContent, matches, contentLengthRx);
		if (matches.size() >= 1)
		{
			_response.contentLength = std::stoull(matches[1].str());
		}
	}
	// Try match content-type
	{
		std::regex_match(headerContent, matches, contentTypeRx);
		if (matches.size() >= 1)
		{
			_response.contentType = matches[1].str();
		}
	}
	// Try match etag
	{
		std::regex_match(headerContent, matches, eTagTypeRx);
		if (matches.size() >= 1)
		{
			_response.eTag = matches[1].str();
		}
	}

	return size * nmemb;
}

size_t AwsRequest::readCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
	AwsRequest* request = static_cast<AwsRequest*>(userdata);
	return request->onRequest(buffer, size, nitems);
}

size_t AwsRequest::writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	AwsRequest* request = static_cast<AwsRequest*>(userdata);
	return request->onResponse(ptr, size, nmemb);
}

size_t AwsRequest::headerCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	AwsRequest* request = static_cast<AwsRequest*>(userdata);
	return request->onHeader(ptr, size, nmemb);
}

} // lib
} // enlighten
