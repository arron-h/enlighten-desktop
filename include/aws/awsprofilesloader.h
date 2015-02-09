#ifndef AWSPROFILESLOADER_H
#define AWSPROFILESLOADER_H

#include <string>

namespace enlighten
{
namespace lib
{
class IAws;
class AwsProfilesLoader
{
public:
	AwsProfilesLoader();

	bool loadProfilesFromPath(const std::string& path, IAws* aws);
};
} // lib
} // enlighten

#endif // AWSPROFILESLOADER_H
