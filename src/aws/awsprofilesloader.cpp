#include "aws/awsprofilesloader.h"
#include "aws/aws.h"
#include "validation.h"
#include "logger.h"

#include "file.h"

#include "rapidjson/document.h"

#include <cassert>

namespace
{
	const char* kAccessProfilesKey  = "accessProfiles";
	const char* kDestinationsKey    = "destinations";

	const char* kAccessKeyIdKey     = "accessKeyId";
	const char* kSecretAccessKeyKey = "secretAccessKey";
	const char* kProfileKey         = "profile";

	const char* kBucketKey          = "bucket";
	const char* kKeyKey             = "key";

	bool loadMember(const rapidjson::Value& v, const std::string& objectName,
			const char* key, std::string& loadedValue)
	{
		rapidjson::Value::ConstMemberIterator valueIterator;
		valueIterator = v.FindMember(key);
		if (valueIterator != v.MemberEnd())
		{
			loadedValue = valueIterator->value.GetString();
			return true;
		}
		else
		{
			enlighten::lib::Logger::get().log(enlighten::lib::Logger::ERROR,
					"Missing %s for object '%s'",
					key, objectName.c_str());
			return false;
		}
	};
}

namespace enlighten
{
namespace lib
{
class AwsProfilesLoaderPrivate
{
public:
	rapidjson::Document _activeDom;
};

AwsProfilesLoader::AwsProfilesLoader() : _impl(new AwsProfilesLoaderPrivate)
{
}

AwsProfilesLoader::~AwsProfilesLoader()
{
	delete _impl;
}

bool AwsProfilesLoader::loadProfilesFromPath(const std::string& path, IAws* aws)
{
	File profileFile(path);
	VALIDATE(profileFile.isValid(), "Profile file does not exist");
	VALIDATE(profileFile.openRead(), "Failed to open profile file for reading");

	uint64_t fileSize = profileFile.fileSize();
	uint8_t* profileFileContents = (uint8_t*)malloc(fileSize+1);

	VALIDATE(profileFile.read(profileFileContents, fileSize) == fileSize,
			"Failed to correctly read profile file");
	profileFileContents[fileSize] = '\0';

	_impl->_activeDom.Parse(reinterpret_cast<char*>(profileFileContents));
	if (_impl->_activeDom.HasParseError())
	{
		Logger::get().log(Logger::ERROR, "Failed to parse AWS profiles Json");
		Logger::get().log(Logger::ERROR, "Json error %d.",
				_impl->_activeDom.GetParseError());

		return false;
	}

	AccessProfileMap accessProfiles;
	DestinationAPMap destinations;
	parseAccessProfiles(accessProfiles);
	parseDestinations(accessProfiles, destinations);

	for (auto destination : destinations)
	{
		const ProfilePair& profilePair = destination.second;
		const std::string identifier = destination.first;
		const AwsDestination& dest = profilePair.second;
		const AwsAccessProfile* ap = profilePair.first;

		aws->initialiseDestinationWithProfile(identifier, *ap, dest);
	}

	free(profileFileContents);

	return true;
}

uint32_t AwsProfilesLoader::parseAccessProfiles(AccessProfileMap& accessProfiles)
{
	rapidjson::Document& d = _impl->_activeDom;

	rapidjson::Value::ConstMemberIterator rootIterator = d.FindMember(kAccessProfilesKey);
	if (rootIterator == d.MemberEnd())
		return 0;

	rapidjson::Value::ConstMemberIterator profilesIterator;
	for (profilesIterator = rootIterator->value.MemberBegin();
			profilesIterator != rootIterator->value.MemberEnd();
			profilesIterator++)
	{
		const rapidjson::Value& v      = profilesIterator->value;
		const std::string& profileName = profilesIterator->name.GetString();

		AwsAccessProfile profile;
		if (!loadMember(v, profileName, kAccessKeyIdKey, profile.accessKeyId))
			continue;

		if (!loadMember(v, profileName, kSecretAccessKeyKey, profile.secretAccessKey))
			continue;

		accessProfiles.insert(std::make_pair(profileName, profile));
	}

	return accessProfiles.size();
}

uint32_t AwsProfilesLoader::parseDestinations(AccessProfileMap& accessProfiles, DestinationAPMap& destinations)
{
	rapidjson::Document& d = _impl->_activeDom;

	rapidjson::Value::ConstMemberIterator rootIterator = d.FindMember(kDestinationsKey);
	if (rootIterator == d.MemberEnd())
		return 0;

	rapidjson::Value::ConstMemberIterator destinationsIterator;
	for (destinationsIterator = rootIterator->value.MemberBegin();
			destinationsIterator != rootIterator->value.MemberEnd();
			destinationsIterator++)
	{
		const rapidjson::Value& v      = destinationsIterator->value;
		const std::string& destinationName = destinationsIterator->name.GetString();

		ProfilePair profilePair;
		AwsDestination& destination = profilePair.second;
		if (!loadMember(v, destinationName, kBucketKey, destination.bucket))
			continue;

		if (!loadMember(v, destinationName, kKeyKey, destination.key))
			continue;

		std::string profileName;
		if (!loadMember(v, destinationName, kProfileKey, profileName))
			continue;

		auto accessProfileIterator = accessProfiles.find(profileName);
		if (accessProfileIterator != accessProfiles.end())
		{
			profilePair.first  = &accessProfileIterator->second;
			destinations.insert(std::make_pair(destinationName, profilePair));
		}
		else
		{
			Logger::get().log(Logger::ERROR, "Failed to find access profile '%s' for destination"
					" '%s'", profileName.c_str(), destinationName.c_str());
		}
	}

	return destinations.size();
}

} // lib
} // enlighten
