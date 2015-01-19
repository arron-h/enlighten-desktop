#include "settings.h"

namespace enlighten
{
namespace lib
{


EnlightenSettings::EnlightenSettings()
{
}

EnlightenSettings::~EnlightenSettings()
{
}

const std::string& EnlightenSettings::get(Setting setting, const std::string& defaultValue)
{
	std::map<Setting, std::string>::iterator it = _settings.find(setting);
	if (it != _settings.end())
	{
		return it->second;
	}
	return defaultValue;
}

void EnlightenSettings::set(Setting setting, const std::string& value)
{
	_settings[setting] = value;
}

int32_t EnlightenSettings::get(Setting setting, int32_t defaultValue)
{
	std::map<Setting, std::string>::iterator it = _settings.find(setting);
	if (it != _settings.end())
	{
		return stoi(it->second);
	}
	return defaultValue;
}

void EnlightenSettings::set(Setting setting, int32_t value)
{
	_settings[setting] = std::to_string(value);
}

double EnlightenSettings::get(Setting setting, double defaultValue)
{
	std::map<Setting, std::string>::iterator it = _settings.find(setting);
	if (it != _settings.end())
	{
		return stod(it->second);
	}
	return defaultValue;
}

void EnlightenSettings::set(Setting setting, double value)
{
	_settings[setting] = std::to_string(value);
}

} // lib
} // enlighten
