#include "platform/linux/paths.h"
#include "validation.h"

namespace enlighten
{
namespace lib
{
Paths::Paths()
{
	_applicationSettings = std::string(getenv("HOME")) + "/.config/enlighten/";
}

const char* Paths::applicationSettings()
{
	return _applicationSettings.c_str();
}

} // lib
} // enlighten
