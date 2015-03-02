#ifndef IPATHS_H
#define IPATHS_H

namespace enlighten
{
namespace lib
{
class IPaths
{
public:
	virtual ~IPaths() {}

	virtual const char* applicationSettings() = 0;
};
} // lib
} // enlighten


#endif // IPATHS_H
