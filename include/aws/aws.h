#ifndef IAWS_H
#define IAWS_H

#include "awsrequest.h"

#include <string>
#include <set>
#include <map>

namespace enlighten
{
namespace lib
{
struct AwsAccessProfile
{
	std::string accessKeyId;
	std::string secretAccessKey;
};

struct AwsDestination
{
	std::string bucket;
	std::string key;
};

class IAws
{
public:
	virtual ~IAws() {}

	virtual IAwsRequest* createRequestForDestination(const std::string& destinationIdentifier) = 0;
	virtual void freeRequest(IAwsRequest* request) = 0;
};

class Aws : public IAws
{
public:
	~Aws();

	bool initialiseDestinationWithProfile(
		const std::string& destinationIdentifier,
		const AwsAccessProfile& accessProfile,
		const AwsDestination& destination);
	void removeDestination(const std::string& destinationIdentifier);
	void removeAllDestinations();

	IAwsRequest* createRequestForDestination(const std::string& destinationIdentifier);
	void freeRequest(IAwsRequest* request);

	static Aws& get();

private:
	Aws();

	std::set<IAwsRequest*> _requests;

	struct AwsPrivateProfile
	{
		AwsDestination destination;
		AwsAccessProfile accessProfile;
	};
	std::map<std::string, AwsPrivateProfile> _destinations;

private:
};
} // lib
} // enlighten


#endif // IAWS_H
