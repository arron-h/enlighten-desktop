#include "aws/aws.h"
#include "validation.h"

#include <curl/curl.h>

#include <cassert>

namespace enlighten
{
namespace lib
{

Aws::Aws()
{
}

Aws::~Aws()
{
	assert(_requests.size() == 0);
	assert(_initialised == false);
}

Aws& Aws::get()
{
	static Aws aws;
	return aws;
}

bool Aws::initialise(const AwsConfig& config)
{
	_config = config;

	if (_initialised)
		return true;

	VALIDATE(curl_global_init(CURL_GLOBAL_DEFAULT) == 0, "Failed to initialise Curl");
	_initialised = true;

	return true;
}

void Aws::shutdown()
{
	curl_global_cleanup();

	_initialised = false;
}

bool Aws::initialiseDestinationWithProfile(
		const std::string& destinationIdentifier,
		const AwsAccessProfile& accessProfile,
		const AwsDestination& destination)
{
	VALIDATE(!accessProfile.accessKeyId.empty(), "No access key provided");
	VALIDATE(!accessProfile.secretAccessKey.empty(), "No secret access key provided");
	VALIDATE(!destination.bucket.empty(), "No bucket name provided");

	auto it = _destinations.find(destinationIdentifier);
	if (it != _destinations.end())
	{
		Logger::get().log(Logger::ERROR, "Destination '%s' already created", destinationIdentifier.c_str());
		return false;
	}

	AwsPrivateProfile privateProfile;
	privateProfile.accessProfile = accessProfile;
	privateProfile.destination   = destination;

	_destinations.insert(std::make_pair(destinationIdentifier, privateProfile));

	return true;
}

void Aws::removeDestination(const std::string& destinationIdentifier)
{
	auto it = _destinations.find(destinationIdentifier);
	if (it != _destinations.end())
	{
		_destinations.erase(it);
	}
}

void Aws::removeAllDestinations()
{
	_destinations.clear();
}

IAwsRequest* Aws::createRequestForDestination(const std::string& destinationIdentifier)
{
	auto it = _destinations.find(destinationIdentifier);

	VALIDATE_AND_RETURN(nullptr, it != _destinations.end(),
		"Destination '%s' has not been initialised", destinationIdentifier.c_str());

	const AwsPrivateProfile& profile = it->second;
	AwsRequest* request = new AwsRequest(&_config, &profile.accessProfile, &profile.destination);
	_requests.insert(request);
	return request;
}

void Aws::freeRequest(IAwsRequest* request)
{
	auto it = _requests.find(request);
	if (it != _requests.end())
	{
		delete *it;
		_requests.erase(it);
	}
}

} // lib
} // enlighten
