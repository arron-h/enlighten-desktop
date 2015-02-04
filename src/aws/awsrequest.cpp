#include "aws/awsrequest.h"
#include "aws/aws.h"
#include "validation.h"

namespace enlighten
{
namespace lib
{

AwsRequest::AwsRequest(const AwsAccessProfile* accessProfile,
		const AwsDestination* destination) :
	_accessProfile(accessProfile), _destination(destination)
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
