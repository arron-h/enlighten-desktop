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
		CachedDatabasePath,
		WatcherPollRate,

		// Probably non-user defined
		PreviewLongestDimension,
		PreviewQuality
	};

public:
	virtual ~IEnlightenSettings() {}

	virtual const std::string& get(Setting setting, const std::string& defaultValue) = 0;
	virtual int32_t get(Setting setting, int32_t defaultValue) = 0;
	virtual double get(Setting setting, double defaultValue) = 0;

	virtual void set(Setting setting, const std::string& value) = 0;
	virtual void set(Setting setting, const int32_t value) = 0;
	virtual void set(Setting setting, const double value) = 0;
};

class EnlightenSettings : public IEnlightenSettings
{
public:
	EnlightenSettings();
	~EnlightenSettings();

	const std::string& get(Setting setting, const std::string& defaultValue);
	int32_t get(Setting setting, int32_t defaultValue);
	double get(Setting setting, double defaultValue);

	void set(Setting setting, const std::string& value);
	void set(Setting setting, const int32_t value);
	void set(Setting setting, const double value);

private:
	std::map<Setting, std::string> _settings;
};
} // lib
} // enlighten

#endif // PREVIEWS_DATABASE_H
