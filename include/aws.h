#ifndef IAWS_H
#define IAWS_H

#include "awsrequest.h"

#include <string>
#include <map>

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
	virtual IAwsRequest* createRequestForBucket(const std::string& bucketName) = 0;
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

	bool initialiseBucketWithConfig(const AwsConfig& config);
	IAwsRequest* createRequestForBucket(const std::string& bucketName);

	static Aws& get();

private:
	Aws();

	std::map<std::string, AwsRequest::AwsPrivateConfig*> _configs;
};
} // lib
} // enlighten


#endif // IAWS_H
