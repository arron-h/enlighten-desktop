#include "aws.h"
#include "validation.h"

namespace enlighten
{
namespace lib
{

AwsRequest::AwsRequest(AwsPrivateConfig* config) : _config(config)
{
}

bool AwsRequest::headObject()
{
	return false;
}

bool AwsRequest::getObject()
{
	return false;
}

bool AwsRequest::putObject()
{
	return false;
}

bool AwsRequest::removeObject()
{
	return false;
}

} // lib
} // enlighten
