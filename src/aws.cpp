#include "aws.h"
#include "validation.h"

namespace enlighten
{
namespace lib
{

Aws::Aws()
{
}

Aws::~Aws()
{
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

bool Aws::initialiseBucketWithConfig(const AwsConfig& config)
{
	AwsRequest::AwsPrivateConfig* privateConfig = new AwsRequest::AwsPrivateConfig;
	privateConfig->accessKeyId     = config.accessKeyId;
	privateConfig->secretAccessKey = config.secretAccessKey;
	privateConfig->region          = config.region;

	_configs.insert(std::make_pair(config.bucketName, privateConfig));
	return true;
}

IAwsRequest* Aws::createRequestForBucket(const std::string& bucketName)
{
	auto it = _configs.find(bucketName);

	VALIDATE_AND_RETURN(nullptr, it != _configs.end(), "Bucket '%s' has not been initialised", bucketName.c_str());

	AwsRequest* request = new AwsRequest(it->second);
	return request;
}

} // lib
} // enlighten
