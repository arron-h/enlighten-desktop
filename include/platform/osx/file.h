#ifndef OSX_FILE_H
#define OSX_FILE_H

#include <pthread.h>
#include <cstdint>
#include <string>

#include "ifile.h"

namespace enlighten
{
namespace lib
{
class File : public IFile
{
public:
	File(const char* file);
	explicit File(const std::string& file);

	bool openRead();
	bool openWrite();

	uint64_t write(uint8_t* bytes, uint32_t size);
	uint64_t read(uint8_t* buffer, uint32_t bufferSize);

	void close();

	const char* filePath() const;
	uint64_t fileSize();

	bool isValid();

	bool duplicate(const char* destination);

	uint64_t lastModificationTime();

private:
	enum Mode
	{
		Read,
		Write,
		None,
	} _mode;

	std::string _file;
	FILE* _handle;
};
} // lib
} // enlighten

#endif // OSX_FILE_H
