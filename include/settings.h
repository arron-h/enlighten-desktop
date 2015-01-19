#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>
#include <string>

namespace enlighten
{
namespace lib
{
class IEnlightenSettings
{
public:
	enum Setting
	{
		CachedDatabasePath
	};

public:
	virtual ~IEnlightenSettings() {}

	virtual const std::string& get(Setting setting, const std::string& defaultValue) = 0;
	virtual void set(Setting setting, const std::string& value) = 0;
};

class EnlightenSettings : public IEnlightenSettings
{
public:
	EnlightenSettings();
	~EnlightenSettings();

	const std::string& get(Setting setting, const std::string& defaultValue);
	void set(Setting setting, const std::string& value);

private:
	std::map<Setting, std::string> _settings;
};
} // lib
} // enlighten

#endif // PREVIEWS_DATABASE_H
