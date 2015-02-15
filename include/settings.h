#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>
#include <string>

namespace enlighten
{
namespace lib
{
const char* applicationSettingsDirectory();

namespace settings
{
	enum UserSetting
	{
		WatcherPollRate,
		PreviewQuality,

		NumUserSettings
	};

	enum StaticSetting
	{
		CachedDatabasePath,
		ProfilesPath,
		PreviewLongestDimension,

		NumStaticSettings
	};
} // settings

class SettingValue
{
public:
	enum BaseType
	{
		kString,
		kInt,
		kDouble
	};

public:
	SettingValue(const std::string* data);
	SettingValue(const int32_t* data);
	SettingValue(const double* data);

	const std::string& toString() const;
	int32_t toInt() const;
	double toDouble() const;

private:
	BaseType _baseType;
	const void* _data;
};

class IUserSettings
{
public:
	virtual ~IUserSettings() {}

	virtual SettingValue get(settings::UserSetting setting, const std::string& defaultValue) const = 0;
	virtual SettingValue get(settings::UserSetting setting, const int32_t& defaultValue) const = 0;
	virtual SettingValue get(settings::UserSetting setting, const double& defaultValue) const = 0;
};

class IStaticSettings
{
public:
	virtual ~IStaticSettings() {}

	virtual SettingValue get(settings::StaticSetting setting) const = 0;
};

class ISettings : public IUserSettings, public IStaticSettings
{
public:
	virtual ~ISettings() {}

	virtual SettingValue get(settings::StaticSetting setting) const = 0;
	virtual SettingValue get(settings::UserSetting setting, const std::string& defaultValue) const = 0;
	virtual SettingValue get(settings::UserSetting setting, const int32_t& defaultValue) const = 0;
	virtual SettingValue get(settings::UserSetting setting, const double& defaultValue) const = 0;
};

class Settings : public ISettings
{
public:
	Settings();
	~Settings();

	bool initialise();

	void set(settings::UserSetting setting, const std::string& value);
	void set(settings::UserSetting setting, const int32_t value);
	void set(settings::UserSetting setting, const double value);

	SettingValue get(settings::StaticSetting setting) const;

	SettingValue get(settings::UserSetting setting, const std::string& defaultValue) const;
	SettingValue get(settings::UserSetting setting, const int32_t& defaultValue) const;
	SettingValue get(settings::UserSetting setting, const double& defaultValue) const;

private:
	Settings(const Settings&);
	Settings& operator=(const Settings&);

	bool loadUserSettings();
	bool loadStaticSettings();

	std::map<settings::UserSetting, std::string> _settings;
	std::map<settings::StaticSetting, std::string> _staticSettings;

	bool _initialised;
};
} // lib
} // enlighten

#endif // SETTINGS_H
