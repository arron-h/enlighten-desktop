#ifndef IFILE_H
#define IFILE_H

namespace enlighten
{
namespace lib
{
class IFile
{
public:
	virtual ~IFile() {}

	virtual bool openRead() = 0;
	virtual bool openWrite() = 0;

	virtual uint64_t write(uint8_t* bytes, uint32_t size) = 0;
	virtual uint64_t read(uint8_t* buffer, uint32_t bufferSize) = 0;

	virtual void close() = 0;

	virtual const char* filePath() const = 0;
	virtual uint64_t fileSize() = 0;

	virtual bool isValid() = 0;

	virtual bool duplicate(const char* destination) = 0;
	virtual uint64_t lastModificationTime() = 0;
};
} // lib
} // enlighten


#endif // IFILE_H
