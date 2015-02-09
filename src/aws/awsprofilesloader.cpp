#include "aws/awsprofilesloader.h"
#include "aws/aws.h"
#include "validation.h"

//#include "rapidjson.h"

#include <cassert>

namespace enlighten
{
namespace lib
{

AwsProfilesLoader::AwsProfilesLoader()
{
}

bool AwsProfilesLoader::loadProfilesFromPath(const std::string& path, IAws* aws)
{
	// TODO - forgot to checkout RapidJson before I got on a plane so hardcoded for now!

	return true;
}

} // lib
} // enlighten
