#include "aws/aws.h"
#include "validation.h"

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
}

Aws& Aws::get()
{
	static Aws aws;
	return aws;
}

bool Aws::initialiseDestinationWithProfile(
		const std::string& destinationIdentifier,
		const AwsAccessProfile& accessProfile,
		const AwsDestination& destination)
{
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

IAwsRequest* Aws::createRequestForDestination(const std::string& destinationIdentifier)
{
	auto it = _destinations.find(destinationIdentifier);

	VALIDATE_AND_RETURN(nullptr, it != _destinations.end(),
		"Destination '%s' has not been initialised", destinationIdentifier.c_str());

	const AwsPrivateProfile& profile = it->second;
	AwsRequest* request = new AwsRequest(&profile.accessProfile, &profile.destination);
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
