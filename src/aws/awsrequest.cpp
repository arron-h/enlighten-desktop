#include "aws/awsrequest.h"
#include "aws/aws.h"
#include "validation.h"
#include "logger.h"

#include "b64.h"

#include <curl/curl.h>

// Apple have deprecated OpenSSL on OSX. Use CommonCrypto instead.
#ifdef __APPLE__
#	define COMMON_DIGEST_FOR_OPENSSL
#	include <CommonCrypto/CommonDigest.h>
#	include <CommonCrypto/CommonHMAC.h>
#else
#	include <openssl/md5.h>
#	include <openssl/sha.h>
#	include <openssl/hmac.h>
#endif


#include <regex>
#include <cstdlib>
#include <cassert>
#include <cstring>

namespace enlighten
{
namespace lib
{
void AwsResponse::dumpBodyToTTY()
{
	std::string stringBody(reinterpret_cast<const char*>(body), bodySize);
	puts(stringBody.c_str());
}

class AwsRequestPrivate
{
public:
	AwsRequestPrivate() : curlHandle(nullptr), curlHeaders(nullptr)
	{
	}

	~AwsRequestPrivate()
	{
		reset();
		if (curlHandle)
		{
			curl_easy_cleanup(curlHandle);
			curlHandle = nullptr;
		}
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
		if (curlHandle)
		{
			curl_easy_reset(curlHandle);
		}
		if (curlHeaders)
		{
			curl_slist_free_all(curlHeaders);
			curlHeaders = nullptr;
		}
	}

	CURL* curlHandle;
	curl_slist* curlHeaders;
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
	VALIDATE(_state == StateIdle, "Request is not in an idle state");

	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "HEAD %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare HEAD request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	CHECK(prepareHeaders("HEAD", nullptr, nullptr, key));

	curl_easy_setopt(handle, CURLOPT_NOBODY, 1l);
	curl_easy_setopt(handle, CURLOPT_HTTPGET, 1l);

	return performRequest(200);
}

bool AwsRequest::getObject(const std::string& key, AwsGet& get)
{
	VALIDATE(_state == StateIdle, "Request is not in an idle state");

	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "GET %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare GET request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	assert(get.buffer == nullptr);
	assert(get.bufferSize == 0);

	_receivedData = 0l;
	_currentWriteData = &get;

	CHECK(prepareHeaders("GET", nullptr, nullptr, key));

	curl_easy_setopt(handle, CURLOPT_HTTPGET, 1l);

	bool success = performRequest(200);
	_currentWriteData = nullptr;

	return success;
}

bool AwsRequest::putObject(const std::string& key, const AwsPut& put)
{
	VALIDATE(_state == StateIdle, "Request is not in an idle state");

	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "PUT %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare PUT request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	_transferredData = 0l;
	_currentReadData = &put;

	uint8_t md5Digest[MD5_DIGEST_LENGTH];
	std::string contentType("application/octet-stream");
	calculateMd5(put, md5Digest);
	CHECK(prepareHeaders("PUT", &contentType, md5Digest, key));

	curl_easy_setopt(handle, CURLOPT_UPLOAD, 1l);
	curl_easy_setopt(handle, CURLOPT_PUT, 1l);
	curl_easy_setopt(handle, CURLOPT_INFILESIZE_LARGE,
		static_cast<curl_off_t>(put.dataSize));

	bool success = performRequest(200);
	_currentReadData = nullptr;

	return success;
}

bool AwsRequest::removeObject(const std::string& key)
{
	VALIDATE(_state == StateIdle, "Request is not in an idle state");

	std::string url = prepareUrl(key);
	Logger::get().log(Logger::INFO, "DELETE %s", url.c_str());
	VALIDATE(prepareRequest(url), "Failed to prepare DELETE request");

	CURL* handle = _privateImpl->curlHandle;
	CHECK(handle)

	CHECK(prepareHeaders("DELETE", nullptr, nullptr, key));

	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");

	return performRequest(204);
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

	_privateImpl->reset();
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

bool AwsRequest::performRequest(long expectedStatusCode)
{
	CURL* handle = _privateImpl->curlHandle;

	CURLcode performResult = curl_easy_perform(handle);

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

	curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &_statusCode);
	_state = StateComplete;

	if (_statusCode != expectedStatusCode)
	{
		Logger::get().log(Logger::ERROR, "Unexpected status code. Expected: %ld got: %ld",
				expectedStatusCode, _statusCode);
	}

	return performResult == CURLE_OK && _statusCode == expectedStatusCode;
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
		_privateImpl->prepareHandle();
	}

	CURL* curlHandle = _privateImpl->curlHandle;
	_state = StateOpening;

	curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curlHandle, CURLOPT_READFUNCTION, readCallback);
	curl_easy_setopt(curlHandle, CURLOPT_READDATA, this);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA , this);
	curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, headerCallback);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEHEADER, this);
	curl_easy_setopt(curlHandle, CURLOPT_PROGRESSFUNCTION, progressCallback);
	curl_easy_setopt(curlHandle, CURLOPT_PROGRESSDATA, this);

	return true;
}

bool AwsRequest::prepareHeaders(const char* httpVerb, const std::string* contentType,
		const uint8_t* md5Digest, const std::string& key)
{
	auto makeKvPair = [] (const char* key, const std::string& value) -> std::string
	{
		return std::string(key) + ": " + value;
	};

	if (contentType)
	{
		VALIDATE(!contentType->empty(), "No content type specified");
		_privateImpl->curlHeaders = curl_slist_append(_privateImpl->curlHeaders,
				makeKvPair("Content-Type", *contentType).c_str());
	}
	if (md5Digest)
	{
		char* allocatedEncode = b64_encode(md5Digest, MD5_DIGEST_LENGTH);
		std::string encodedString(allocatedEncode);
		free(allocatedEncode);

		_privateImpl->curlHeaders = curl_slist_append(_privateImpl->curlHeaders,
				makeKvPair("Content-MD5", encodedString).c_str());
	}

	VALIDATE(_accessProfile, "No access profile provided");
	VALIDATE(_destination, "No destination provided");

	VALIDATE(!_accessProfile->accessKeyId.empty(), "No access key ID");
	VALIDATE(!_accessProfile->secretAccessKey.empty(), "No secret access key");
	VALIDATE(!_destination->bucket.empty(), "No bucket name");

	// Auth
	std::string authToken = "AWS " + _accessProfile->accessKeyId + ":";
	authToken += generateAuthenticationSignature(httpVerb, contentType, md5Digest, key);
	_privateImpl->curlHeaders = curl_slist_append(_privateImpl->curlHeaders,
			makeKvPair("Authorization", authToken).c_str());

	curl_easy_setopt(_privateImpl->curlHandle, CURLOPT_HTTPHEADER, _privateImpl->curlHeaders);

	return true;
}

void AwsRequest::calculateMd5(const AwsPut& put, uint8_t* digest)
{
	MD5_CTX md5Ctx;
	MD5_Init(&md5Ctx);
	MD5_Update(&md5Ctx, put.data, put.dataSize);
	MD5_Final(digest, &md5Ctx);
}

std::string AwsRequest::generateAuthenticationSignature(const char* httpVerb,
		const std::string* contentType, const uint8_t* md5Digest, const std::string& key)
{
	std::string stringToSign;
	stringToSign += httpVerb;
	stringToSign += "\n";
	if (md5Digest)
	{
		stringToSign += reinterpret_cast<const char*>(md5Digest);	// AH: Not sure this is correct? Maybe the b64 encoded variant?
	}
	stringToSign += "\n";
	if (contentType)
	{
		stringToSign += *contentType;
	}
	stringToSign += "\n";

	std::locale currentLocal;
	std::locale::global(std::locale("en_US"));

	char timeBuffer[128];
	std::time_t timeNow = std::time(NULL);
	std::strftime(timeBuffer, sizeof(timeBuffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&timeNow));

	std::locale::global(currentLocal);

	stringToSign += timeBuffer;
	stringToSign += "\n";
	stringToSign += "/" + _destination->bucket + "/" + _destination->key + "/" + key;

	uint8_t shaDigest[SHA_DIGEST_LENGTH];
#ifdef __APPLE__
	CCHmac(kCCHmacAlgSHA1,
		_accessProfile->secretAccessKey.c_str(),
		_accessProfile->secretAccessKey.length(),
		stringToSign.c_str(),
		stringToSign.length(),
		shaDigest);
#else
	HMAC(EVP_sha1(),
		_accessProfile->secretAccessKey.c_str(),
		_accessProfile->secretAccessKey.length(),
		stringToSign.c_str(),
		stringToSign.length(),
		shaDigest,
		NULL);
#endif

	// Base64 encode this
	char* allocatedEncode = b64_encode(shaDigest, SHA_DIGEST_LENGTH);
	std::string encodedString = allocatedEncode;
	free(allocatedEncode);

	return encodedString;
}

AwsRequest::RequestState AwsRequest::state()
{
	return _state;
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

int32_t AwsRequest::onProgress(uint64_t dltotal, uint64_t dlnow, uint64_t ultotal, uint64_t ulnow)
{
	if (_cancel)
	{
		return -1;
	}

	return 0;
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

size_t AwsRequest::progressCallback(void *clientp, uint64_t dltotal, uint64_t dlnow, uint64_t ultotal, uint64_t ulnow)
{
	AwsRequest* request = static_cast<AwsRequest*>(clientp);
	return request->onProgress(dltotal, dlnow, ultotal, ulnow);
}

} // lib
} // enlighten
