#ifndef JPEG_H
#define JPEG_H

#include <cstdint>
#include <string>
#include <cstdint>

namespace enlighten
{
namespace lib
{
class IJpeg
{
public:
	IJpeg() {}
	virtual ~IJpeg() {}

	virtual bool decompress() = 0;
	virtual bool compress(uint32_t qualityLevel) = 0;

	virtual uint32_t components() const = 0;
	virtual uint32_t width() const = 0;
	virtual uint32_t height() const = 0;
	virtual const uint8_t* rawBytes() const = 0;

	virtual bool fromRawBytes(uint8_t* bytes, uint32_t width, uint32_t height, uint32_t components) = 0;
	virtual const uint8_t* compressedData(uint32_t& size) const = 0;

	virtual bool writeToFile(const char* filePath) = 0;
};

class Jpeg : public IJpeg
{
public:
	Jpeg();
	Jpeg(uint8_t* bytes, uint32_t size, bool retain);
	virtual ~Jpeg();

	bool decompress();
	bool compress(uint32_t qualityLevel);

	uint32_t components() const;
	uint32_t width() const;
	uint32_t height() const;
	const uint8_t* rawBytes() const;

	bool fromRawBytes(uint8_t* bytes, uint32_t width, uint32_t height, uint32_t components);
	const uint8_t* compressedData(uint32_t& size) const;

	bool writeToFile(const char* filePath);

private:
	bool _retainedCompressedData;
	uint8_t* _compressedBytes;
	uint32_t _compressedSize;

	uint8_t* _decompressedBytes;
	uint32_t _width;
	uint32_t _height;
	uint32_t _components;
};
} // lib
} // enlighten

#endif // JPEG_H
