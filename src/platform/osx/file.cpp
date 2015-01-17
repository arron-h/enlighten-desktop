#include "platform/osx/file.h"
#include "validation.h"

#include <sys/stat.h>
#include <unistd.h>

namespace enlighten
{
namespace lib
{

File::File(const char* filePath) : _mode(None), _file(filePath), _handle(nullptr)
{
}

File::File(const std::string& filePath) : _mode(None), _file(filePath), _handle(nullptr)
{
}

bool File::openRead()
{
	VALIDATE(_mode != Write, "File already opened for writing");
	if (_mode == Read)
	{
		return true; // already opened for reading
	}

	_handle = fopen(_file.c_str(), "rb");
	VALIDATE(_handle, "Failed to open file '%s' for reading", _file.c_str());

	_mode = Read;

	return true;
}

bool File::openWrite()
{
	VALIDATE(_mode != Read, "File already opened for reading");
	if (_mode == Write)
	{
		return true; // already opened for writing
	}

	_handle = fopen(_file.c_str(), "wb");
	VALIDATE(_handle, "Failed to open file '%s' for writing", _file.c_str());

	_mode = Write;

	return true;
}

uint64_t File::write(uint8_t* bytes, uint32_t size)
{
	VALIDATE(_handle && _mode == Write, "File is not opened for writing");

	return fwrite(bytes, 1, size, _handle);
}

uint64_t File::read(uint8_t* buffer, uint32_t bufferSize)
{
	VALIDATE(_handle && _mode == Read, "File is not opened for reading");

	return fread(buffer, 1, bufferSize, _handle);
}

void File::close()
{
	if (_handle)
		fclose(_handle);

	_handle = nullptr;
	_mode = None;
}

const char* File::filePath() const
{
	return _file.c_str();
}

bool File::isValid()
{
	struct stat attrib;
	return stat(_file.c_str(), &attrib) == 0;
}

uint64_t File::fileSize()
{
	struct stat attrib;

	if (_handle)
	{
		fstat(fileno(_handle), &attrib);
	}
	else
	{
		stat(_file.c_str(), &attrib);
	}

	return static_cast<uint64_t>(attrib.st_size);
}

bool File::duplicate(const char* destination)
{
	if (!isValid())
		return false;

	uint8_t buffer[BUFSIZ];
	uint64_t bytesRead;

	File destFile(destination);

	CHECK(openRead());
	CHECK(destFile.openWrite());

	while ((bytesRead = read(buffer, BUFSIZ)))
	{
        destFile.write(buffer, bytesRead);
    }

	close();
	destFile.close();

	return true;
}

uint64_t File::lastModificationTime()
{
	struct stat attrib;
	stat(_file.c_str(), &attrib);
	__darwin_time_t lastModTime = attrib.st_mtimespec.tv_sec;

	return static_cast<uint64_t>(lastModTime);
}
} // lib
} // enlighten
