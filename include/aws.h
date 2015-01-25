#ifndef IAWS_H
#define IAWS_H

#include "awsrequest.h"

#include <string>
#include <map>
#include <set>

namespace enlighten
{
namespace lib
{
struct AwsPrivateConfig;
class AwsRequest;

class IAws
{
public:
	virtual ~IAws() {}
	virtual IAwsRequest* createRequestForProfile(const std::string& bucketName) = 0;
	virtual void freeRequest(IAwsRequest* request) = 0;
};

class Aws : public IAws
{
public:
	struct AwsConfig
	{
		const char* accessKeyId;
		const char* secretAccessKey;
		const char* region;
		const char* bucketName;
	};
public:
	~Aws();

	bool initialiseBucketWithProfile(const AwsConfig& config);
	IAwsRequest* createRequestForProfile(const std::string& bucketName);
	void freeRequest(IAwsRequest* request);

	static Aws& get();

private:
	Aws();

	std::map<std::string, AwsRequest::AwsPrivateConfig*> _configs;
	std::set<IAwsRequest*> _requests;
};
} // lib
} // enlighten


#endif // IAWS_H
