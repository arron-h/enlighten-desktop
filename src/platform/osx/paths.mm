#include "platform/osx/paths.h"
#include "validation.h"

#include <Foundation/Foundation.h>

namespace enlighten
{
namespace lib
{
Paths::Paths()
{
	NSFileManager* manager = [NSFileManager defaultManager];
	NSURL* url = [[manager URLsForDirectory:NSApplicationSupportDirectory
		inDomains:NSUserDomainMask] objectAtIndex:0];

	NSString* urlPath = [[url path] stringByAppendingPathComponent:@"enlighten/"];
	
	_applicationSettings = [urlPath UTF8String];
	
	[url release];
	[urlPath release];
}

const char* Paths::applicationSettings()
{
	return _applicationSettings.c_str();
}

} // lib
} // enlighten
