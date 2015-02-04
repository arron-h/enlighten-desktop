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

struct AwsAccessProfile;
struct AwsDestination;
class AwsRequest : public IAwsRequest
{
public:
	AwsRequest(const AwsAccessProfile* accessProfile,
		const AwsDestination* destination);

	bool headObject();
	bool getObject();
	bool putObject();
	bool removeObject();

private:
	const AwsAccessProfile* _accessProfile;
	const AwsDestination* _destination;
};
} // lib
} // enlighten


#endif // IAWSREQUEST_H
