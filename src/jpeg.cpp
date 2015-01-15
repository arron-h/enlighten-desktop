#include "jpeg.h"
#include "validation.h"

#include <jpeglib.h>
#include <cstdlib>

namespace enlighten
{
namespace lib
{
namespace
{
	const uint32_t kBufferAllocationSize = 0x8000;

	struct JpegMemoryDestination
	{
		jpeg_destination_mgr manager;

		uint8_t** _byteBuffer;
		uint32_t* _bufferSize;
	};

	static void initialiseDestination(j_compress_ptr cinfo)
	{
		JpegMemoryDestination* dest = reinterpret_cast<JpegMemoryDestination*>(
			cinfo->dest);

		*(dest->_byteBuffer) = (uint8_t*)malloc(kBufferAllocationSize);
		*(dest->_bufferSize) = kBufferAllocationSize;

		cinfo->dest->next_output_byte = *(dest->_byteBuffer);
		cinfo->dest->free_in_buffer = kBufferAllocationSize;
	}

	static void terminateDestination(j_compress_ptr cinfo)
	{
		JpegMemoryDestination* dest = reinterpret_cast<JpegMemoryDestination*>(
			cinfo->dest);

		*(dest->_bufferSize) -= cinfo->dest->free_in_buffer;
	}

	static int flushOutputBuffer(j_compress_ptr cinfo)
	{
		JpegMemoryDestination* dest = reinterpret_cast<JpegMemoryDestination*>(
			cinfo->dest);

		uint32_t oldBufferSize = *(dest->_bufferSize);
		uint32_t newBufferSize = oldBufferSize + kBufferAllocationSize;

		// Reallocate and copy old contents
		uint8_t* oldByteBuffer = *(dest->_byteBuffer);
		uint8_t* newByteBuffer = (uint8_t*)malloc(newBufferSize);
		memcpy(newByteBuffer, oldByteBuffer, oldBufferSize);

		*(dest->_byteBuffer) = newByteBuffer;
		*(dest->_bufferSize) = newBufferSize;

		free(oldByteBuffer);

		cinfo->dest->next_output_byte = *(dest->_byteBuffer) + oldBufferSize;
		cinfo->dest->free_in_buffer = kBufferAllocationSize;

		return 1;
	}

}

Jpeg::Jpeg() : _retainedCompressedData(false), _compressedBytes(nullptr),
	_compressedSize(0), _decompressedBytes(nullptr), _width(0), _height(0),
	_components(0)
{
}

Jpeg::Jpeg(uint8_t* bytes, uint32_t size, bool retain) : _retainedCompressedData(retain),
	_decompressedBytes(nullptr), _width(0), _height(0), _components(0)
{
	if (retain)
	{
		_compressedBytes = (uint8_t*)malloc(size);
		_compressedSize  = size;
		memcpy(_compressedBytes, bytes, size);
	}
	else
	{
		_compressedBytes = bytes;
		_compressedSize = size;
	}
}

Jpeg::~Jpeg()
{
	if (_decompressedBytes)
	{
		free(_decompressedBytes);
		_decompressedBytes = nullptr;
	}

	if (_retainedCompressedData && _compressedBytes)
	{
		free(_compressedBytes);
		_compressedBytes = nullptr;
	}
}

bool Jpeg::decompress()
{
	VALIDATE(_compressedBytes, "No compressed data set");
	VALIDATE(_compressedSize > 0, "Compressed data size is 0");
	VALIDATE(_decompressedBytes == nullptr, "Decompressed image already set");

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, _compressedBytes, _compressedSize);

	VALIDATE(jpeg_read_header(&cinfo, TRUE), "Failed to read Jpeg header");

	jpeg_start_decompress(&cinfo);

	_width  = cinfo.output_width;
	_height = cinfo.output_height;
	_components = cinfo.output_components;

	size_t decompressedSize = _width * _height * _components;
	size_t rowStride        = _width * _components;
	_decompressedBytes = (uint8_t*)malloc(decompressedSize);

	uint8_t* scanlineBuffer[1];
	while (cinfo.output_scanline < cinfo.output_height)
	{
		*scanlineBuffer = _decompressedBytes + cinfo.output_scanline * rowStride;
		jpeg_read_scanlines(&cinfo, scanlineBuffer, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return true;
}

bool Jpeg::compress(uint32_t qualityLevel)
{
	VALIDATE(qualityLevel >= 0 && qualityLevel <= 100, "Invalid quality level");
	VALIDATE(_decompressedBytes, "No RGB bytes set");
	VALIDATE(_compressedBytes == nullptr, "Compressed image already set");

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JpegMemoryDestination memoryDestination;
	memoryDestination._byteBuffer = &_compressedBytes;
	memoryDestination._bufferSize = &_compressedSize;

	cinfo.err  = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	cinfo.dest = reinterpret_cast<jpeg_destination_mgr*>(&memoryDestination);
	cinfo.dest->init_destination    = initialiseDestination;
	cinfo.dest->empty_output_buffer = flushOutputBuffer;
	cinfo.dest->term_destination    = terminateDestination;

	cinfo.image_width      = _width;
	cinfo.image_height     = _height;
	cinfo.input_components = _components;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, qualityLevel, true);

	jpeg_start_compress(&cinfo, true);

	_retainedCompressedData = true; // _compressedBytes will have been allocated

	uint32_t rowStride = _width * _components;

	uint8_t* scanlineBuffer[1];
	while (cinfo.next_scanline < cinfo.image_height)
	{
		*scanlineBuffer = _decompressedBytes + cinfo.next_scanline * rowStride;
		jpeg_write_scanlines(&cinfo, scanlineBuffer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return true;
}

uint32_t Jpeg::components() const
{
	return _components;
}

uint32_t Jpeg::width() const
{
	return _width;
}

uint32_t Jpeg::height() const
{
	return _height;
}

const uint8_t* Jpeg::rawBytes() const
{
	return _decompressedBytes;
}

bool Jpeg::fromRawBytes(uint8_t* bytes, uint32_t width, uint32_t height, uint32_t components)
{
	VALIDATE(bytes, "input bytes must not be nullptr");
	VALIDATE(width  > 0, "width must be greater than 0");
	VALIDATE(height > 0, "height must be greater than 0");
	VALIDATE(components == 3, "components must be 3");

	if (_decompressedBytes)
		free(_decompressedBytes);

	uint32_t bufferSize = width*height*components;
	_decompressedBytes = (uint8_t*)malloc(bufferSize);
	memcpy(_decompressedBytes, bytes, bufferSize);

	_width = width;
	_height = height;
	_components = components;
	return true;
}

bool Jpeg::writeToFile(const char* filePath)
{
	VALIDATE(filePath, "filePath is not valid");

	FILE* fileHandle = fopen(filePath, "wb");
	VALIDATE(fileHandle, "Failed to open '%s' for writing", filePath);

	size_t bytesWritten = fwrite(_compressedBytes, 1, _compressedSize, fileHandle);
	VALIDATE(bytesWritten == _compressedSize, "Failed to correctly write '%s'", filePath);

	fclose(fileHandle);

	return true;
}

const uint8_t* Jpeg::compressedData(uint32_t& size) const
{
	size = _compressedSize;
	return _compressedBytes;
}
} // lib
} // enlighten
