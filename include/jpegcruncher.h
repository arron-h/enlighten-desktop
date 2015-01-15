#ifndef JPEG_CRUNCHER_H
#define JPEG_CRUNCHER_H

#include <cstdint>
#include <string>

namespace enlighten
{
namespace lib
{
class IJpeg;
class JpegCruncher
{
public:
	JpegCruncher(IJpeg* sourceJpeg, IJpeg* targetJpeg);

	bool reencodeJpeg(uint32_t longestDimension, int32_t qualityLevel);
private:
	bool rescaleBuffer(const uint8_t* sourceBuffer, uint32_t sourceWidth, uint32_t sourceHeight,
		uint32_t components, uint8_t* targetBuffer, uint32_t targetWidth, uint32_t targetHeight);

	IJpeg* _sourceJpeg;
	IJpeg* _targetJpeg;
};
} // lib
} // enlighten

#endif // JPEG_CRUNCHER_H
