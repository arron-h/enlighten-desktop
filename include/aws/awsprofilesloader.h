#ifndef AWSPROFILESLOADER_H
#define AWSPROFILESLOADER_H

#include <string>
#include <map>

namespace enlighten
{
namespace lib
{
struct AwsAccessProfile;
struct AwsDestination;

class IAws;
class AwsProfilesLoaderPrivate;
class AwsProfilesLoader
{
public:
	AwsProfilesLoader();
	~AwsProfilesLoader();

	bool loadProfilesFromPath(const std::string& path, IAws* aws);

private:
	typedef std::pair<AwsAccessProfile*,AwsDestination> ProfilePair;
	typedef std::map<std::string, AwsAccessProfile> AccessProfileMap;
	typedef std::map<std::string, ProfilePair> DestinationAPMap;

	uint32_t parseAccessProfiles(AccessProfileMap& accessProfiles);
	uint32_t parseDestinations(AccessProfileMap& accessProfiles, DestinationAPMap& destinations);

	AwsProfilesLoaderPrivate* _impl;
};
} // lib
} // enlighten

#endif // AWSPROFILESLOADER_H
