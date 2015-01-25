#include "aws.h"
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

	auto it = _configs.begin();
	for (; it != _configs.end(); it++)
	{
		delete it->second;
	}
}

Aws& Aws::get()
{
	static Aws aws;
	return aws;
}

bool Aws::initialiseBucketWithProfile(const AwsConfig& config)
{
	AwsRequest::AwsPrivateConfig* privateConfig = new AwsRequest::AwsPrivateConfig;
	privateConfig->accessKeyId     = config.accessKeyId;
	privateConfig->secretAccessKey = config.secretAccessKey;
	privateConfig->region          = config.region;

	_configs.insert(std::make_pair(config.bucketName, privateConfig));
	return true;
}

IAwsRequest* Aws::createRequestForProfile(const std::string& profileName)
{
	auto it = _configs.find(profileName);

	VALIDATE_AND_RETURN(nullptr, it != _configs.end(), "Profile '%s' has not been initialised", profileName.c_str());

	AwsRequest* request = new AwsRequest(it->second);
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
