#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "cachedpreviews.h"
#include "settings.h"

using namespace enlighten::lib;

namespace
{
}

TEST(SettingsTest, ShouldConstruct)
{
	Settings settings;
}

TEST(SettingsTest, ShouldInitialise)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
}

TEST(SettingsTest, ShouldGetStaticSetting)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	EXPECT_NO_FATAL_FAILURE(settings.get(settings::PreviewLongestDimension));
}

TEST(SettingsDeathTest, ShouldFailGetInvalidStaticSetting)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	EXPECT_DEATH(settings.get(settings::NumStaticSettings), "");
}

TEST(SettingsTest, ShouldGetUserSetting_String)
{
	// Not tested as we don't have any 'string' user settings
}

TEST(SettingsTest, ShouldGetUserSetting_Int)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	SettingValue v = settings.get(settings::WatcherPollRate, 100);
	EXPECT_GT(v.toInt(), 0);
}

TEST(SettingsTest, ShouldGetUserSetting_Double)
{
	// Not tested as we don't have any 'double' user settings
}

TEST(SettingsDeathTest, ShouldFailGetInvalidUserSetting)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	EXPECT_DEATH(settings.get(settings::NumUserSettings, "blah"), "");
}

TEST(SettingsTest, ShouldSetUserSetting_String)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	settings.set(settings::PreviewQuality, "good"); // This is not how PreviewQuality works and is only for testing
	EXPECT_EQ("good", settings.get(settings::PreviewQuality, "ok").toString());
}

TEST(SettingsTest, ShouldSetUserSetting_Int)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	settings.set(settings::WatcherPollRate, 367);
	EXPECT_EQ(367, settings.get(settings::WatcherPollRate, 10).toInt());
}

TEST(SettingsTest, ShouldSetUserSetting_Double)
{
	Settings settings;
	EXPECT_TRUE(settings.initialise());
	settings.set(settings::PreviewQuality, 67.9); // PreviewQuality should actually be int
	EXPECT_EQ(67.9, settings.get(settings::PreviewQuality, 0.0).toDouble());
}

TEST(SettingValueTest, ShouldSetStringReturnString)
{
	std::string value("my value");
	SettingValue v(&value);
	EXPECT_EQ("my value", v.toString());
}

TEST(SettingValueTest, ShouldSetStringReturnInt)
{
	std::string value("1056");
	SettingValue v(&value);
	EXPECT_EQ(1056, v.toInt());
}

TEST(SettingValueTest, ShouldSetStringReturnDouble)
{
	std::string value("443.456");
	SettingValue v(&value);
	EXPECT_EQ(443.456, v.toDouble());
}

TEST(SettingValueTest, ShouldSetIntReturnInt)
{
	int32_t value = 1056;
	SettingValue v(&value);
	EXPECT_EQ(1056, v.toInt());
}

TEST(SettingValueTest, ShouldSetIntReturnDouble)
{
	int32_t value = 23;
	SettingValue v(&value);
	EXPECT_EQ(23.0, v.toDouble());
}

TEST(SettingValueTest, ShouldSetDoubleReturnDouble)
{
	double value = 567.12;
	SettingValue v(&value);
	EXPECT_EQ(567.12, v.toDouble());
}

TEST(SettingValueTest, ShouldSetDoubleReturnInt)
{
	double value = 567.12;
	SettingValue v(&value);
	EXPECT_EQ(567, v.toInt());
}

TEST(SettingValueDeathTest, ShouldFailReturnStringIfSourceNotString)
{
	double doubleValue = 567.12;
	SettingValue vd(&doubleValue);
	EXPECT_DEATH(vd.toString(), "");

	int32_t intValue = 12;
	SettingValue vi(&intValue);
	EXPECT_DEATH(vi.toString(), "");
}
