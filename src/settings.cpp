#include "settings.h"
#include <cassert>

namespace
{
	const char* kLongestDimensionDefault = "220";
}

namespace enlighten
{
namespace lib
{
SettingValue::SettingValue(const std::string* data) :
	_baseType(kString), _data(data)
{
}

SettingValue::SettingValue(const int32_t* data) :
	_baseType(kInt), _data(data)
{
}

SettingValue::SettingValue(const double* data) :
	_baseType(kDouble), _data(data)
{
}

const std::string& SettingValue::toString() const
{
	assert(_baseType == kString);

	const std::string* value = static_cast<const std::string*>(_data);
	return *value;
}

int32_t SettingValue::toInt() const
{
	if (_baseType == kString)
	{
		const std::string* value = static_cast<const std::string*>(_data);
		return stoi(*value);
	}
	else if (_baseType == kInt)
	{
		return *static_cast<const int32_t*>(_data);
	}
	else if (_baseType == kDouble)
	{
		return static_cast<int32_t>(*static_cast<const double*>(_data));
	}

	assert(!"Unknown base type");
	return 0;
}

double SettingValue::toDouble() const
{
	if (_baseType == kString)
	{
		const std::string* value = static_cast<const std::string*>(_data);
		return stod(*value);
	}
	else if (_baseType == kInt)
	{
		return static_cast<double>(*static_cast<const int32_t*>(_data));
	}
	else if (_baseType == kDouble)
	{
		return *static_cast<const double*>(_data);
	}

	assert(!"Unknown base type");
	return 0;
}

Settings::Settings() : _initialised(false)
{
}

Settings::~Settings()
{
}

bool Settings::initialise()
{
	// Load
	bool initialisedCorrectly =
		loadStaticSettings() && loadUserSettings();

	if (initialisedCorrectly)
		_initialised = true;

	return initialisedCorrectly;
}

bool Settings::loadUserSettings()
{
	return true;
}

bool Settings::loadStaticSettings()
{
	_staticSettings[settings::PreviewLongestDimension] = kLongestDimensionDefault;
	_staticSettings[settings::CachedDatabasePath] = applicationSettingsDirectory();
	_staticSettings[settings::ProfilesPath] = applicationSettingsDirectory();

	return true;
}

SettingValue Settings::get(settings::StaticSetting setting) const
{
	assert(setting >= 0 && setting < settings::NumStaticSettings);

	SettingValue v(&_staticSettings.at(setting));
	return v;
}

SettingValue Settings::get(settings::UserSetting setting, const std::string& defaultValue) const
{
	assert(setting >= 0 && setting < settings::NumUserSettings);

	std::map<settings::UserSetting, std::string>::const_iterator it = _settings.find(setting);
	if (it != _settings.end())
	{
		return SettingValue(&it->second);
	}
	return SettingValue(&defaultValue);
}

void Settings::set(settings::UserSetting setting, const std::string& value)
{
	_settings[setting] = value;
}

SettingValue Settings::get(settings::UserSetting setting, const int32_t& defaultValue) const
{
	assert(setting >= 0 && setting < settings::NumUserSettings);

	std::map<settings::UserSetting, std::string>::const_iterator it = _settings.find(setting);
	if (it != _settings.end())
	{
		return SettingValue(&it->second);
	}
	return SettingValue(&defaultValue);
}

void Settings::set(settings::UserSetting setting, int32_t value)
{
	_settings[setting] = std::to_string(value);
}

SettingValue Settings::get(settings::UserSetting setting, const double& defaultValue) const
{
	assert(setting >= 0 && setting < settings::NumUserSettings);

	std::map<settings::UserSetting, std::string>::const_iterator it = _settings.find(setting);
	if (it != _settings.end())
	{
		return SettingValue(&it->second);
	}
	return SettingValue(&defaultValue);
}

void Settings::set(settings::UserSetting setting, double value)
{
	_settings[setting] = std::to_string(value);
}

} // lib
} // enlighten
