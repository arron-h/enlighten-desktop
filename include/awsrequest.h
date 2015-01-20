#ifndef IAWSREQUEST_H
#define IAWSREQUEST_H

#include <string>

namespace enlighten
{
namespace lib
{
class IAwsRequest
{
public:
	virtual ~IAwsRequest() {}

	virtual bool headObject() = 0;
	virtual bool getObject() = 0;
	virtual bool putObject() = 0;
	virtual bool removeObject() = 0;
};

class AwsRequest : public IAwsRequest
{
public:
	bool headObject();
	bool getObject();
	bool putObject();
	bool removeObject();

private:
	struct AwsPrivateConfig
	{
		std::string accessKeyId;
		std::string secretAccessKey;
		std::string region;
	};

private:
	AwsRequest(AwsPrivateConfig* config);

private:
	AwsPrivateConfig* _config;

	friend class Aws;
};
} // lib
} // enlighten


#endif // IAWSREQUEST_H
