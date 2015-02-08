#ifndef IAWSREQUEST_H
#define IAWSREQUEST_H

#include <string>

namespace enlighten
{
namespace lib
{
struct AwsResponse
{
	uint64_t contentLength;
	std::string contentType;
	std::string eTag;

	uint8_t* body;
	uint64_t bodySize;

	AwsResponse() : contentLength(0), body(nullptr), bodySize(0) {}
	void dumpBodyToTTY();
};

struct AwsGet
{
	uint8_t* buffer;
	uint64_t bufferSize;
};

struct AwsPut
{
	const uint8_t* data;
	uint64_t dataSize;
};

class IAwsRequest
{
public:
	virtual ~IAwsRequest() {}

	virtual bool headObject(const std::string& key) = 0;
	virtual bool getObject(const std::string& key, AwsGet& get) = 0;
	virtual bool putObject(const std::string& key, const AwsPut& put) = 0;
	virtual bool removeObject(const std::string& key) = 0;

	virtual void cancel() = 0;
	virtual void reset() = 0;

	virtual const AwsResponse* response() = 0;
	virtual int32_t statusCode() = 0;
};

struct AwsConfig;
struct AwsAccessProfile;
struct AwsDestination;

class AwsRequestPrivate;
class AwsRequest : public IAwsRequest
{
public:
	enum RequestState
	{
		StateIdle,
		StateOpening,
		StateCancelled,
		StateReceiving,
		StateTransferring,
		StateComplete
	};

public:
	AwsRequest(const AwsConfig* config,
		const AwsAccessProfile* accessProfile,
		const AwsDestination* destination);
	~AwsRequest();

	bool headObject(const std::string& key);
	bool getObject(const std::string& key, AwsGet& get);
	bool putObject(const std::string& key, const AwsPut& put);
	bool removeObject(const std::string& key);

	void cancel();
	void reset();
	RequestState state();

	const AwsResponse* response();
	int32_t statusCode();

private:
	AwsRequest(const AwsRequest&);
	void operator=(const AwsRequest&);

	static size_t readCallback(char *buffer, size_t size, size_t nitems, void *userdata);
	static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t headerCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t progressCallback(void *clientp, uint64_t dltotal, uint64_t dlnow, uint64_t ultotal, uint64_t ulnow);

	bool performRequest(long expectedStatusCode);

	bool prepareRequest(const std::string& key);
	std::string prepareUrl(const std::string& key);
	bool prepareHeaders(const char* httpVerb, const std::string* contentType,
			const uint8_t* md5Digest, const std::string& key);

	void calculateMd5(const AwsPut& put, uint8_t* digest);
	std::string generateAuthenticationSignature(const char* httpVerb,
			const std::string* contentType, const uint8_t* md5Digest,
			const std::string& key);

	int32_t onRequest(char *buffer, size_t size, size_t nitems);
	int32_t onResponse(char *ptr, size_t size, size_t nmemb);
	int32_t onHeader(char *ptr, size_t size, size_t nmemb);
	int32_t onProgress(uint64_t dltotal, uint64_t dlnow, uint64_t ultotal, uint64_t ulnow);

	const AwsConfig* _config;
	const AwsAccessProfile* _accessProfile;
	const AwsDestination* _destination;

	AwsRequestPrivate* _privateImpl;
	AwsResponse _response;
	long _statusCode;

	volatile bool _cancel;

	// Read/write members
	const AwsPut* _currentReadData;
	uint64_t _transferredData;

	AwsGet* _currentWriteData;
	uint64_t _receivedData;

	RequestState _state;
};
} // lib
} // enlighten


#endif // IAWSREQUEST_H
