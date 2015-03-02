#ifndef OSX_PATHS_H
#define OSX_PATHS_H

#include "ipaths.h"
#include <string>

namespace enlighten
{
namespace lib
{
class Paths : public IPaths
{
public:
	Paths();

	const char* applicationSettings();

private:
	std::string _applicationSettings;
};
} // lib
} // enlighten

#endif // OSX_PATHS_H
