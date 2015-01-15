#include "jpegcruncher.h"
#include "validation.h"
#include "jpeg.h"

#include <jpeglib.h>
#include <cstdlib>
#include <cstring>

namespace enlighten
{
namespace lib
{
JpegCruncher::JpegCruncher(IJpeg* sourceJpeg, IJpeg* targetJpeg) :
	_sourceJpeg(sourceJpeg), _targetJpeg(targetJpeg)
{
}

bool JpegCruncher::reencodeJpeg(uint32_t longestDimension, int32_t qualityLevel)
{
	VALIDATE(_sourceJpeg, "Source Jpeg is invalid");
	VALIDATE(_targetJpeg, "Target Jpeg is invalid");

	VALIDATE(_sourceJpeg->decompress(), "Failed to decompress Jpeg");

	// Scale the image down
	uint32_t targetWidth, targetHeight;
	uint32_t sourceWidth  = _sourceJpeg->width();
	uint32_t sourceHeight = _sourceJpeg->height();
	uint32_t components   = _sourceJpeg->components();
	if (_sourceJpeg->width() >= _sourceJpeg->height())
	{
		float aspectRatio = static_cast<float>(_sourceJpeg->height()) / _sourceJpeg->width();
		targetWidth  = longestDimension;
		targetHeight = longestDimension * aspectRatio;
	}
	else
	{
		float aspectRatio = static_cast<float>(_sourceJpeg->width()) / _sourceJpeg->height();
		targetHeight = longestDimension;
		targetWidth  = longestDimension * aspectRatio;
	}

	const uint8_t* sourceBytes = _sourceJpeg->rawBytes();
	uint8_t* targetBytes = (uint8_t*)malloc(targetWidth * targetHeight * components);

	bool rescaleSuccessful = rescaleBuffer(sourceBytes, sourceWidth, sourceHeight, components,
		targetBytes, targetWidth, targetHeight);

	VALIDATE(rescaleSuccessful, "Failed to resize preview jpeg file.");

	_targetJpeg->fromRawBytes(targetBytes, targetWidth, targetHeight, components);

	VALIDATE(_targetJpeg->compress(qualityLevel), "Failed to compress Jpeg");

	return true;
}

bool JpegCruncher::rescaleBuffer(const uint8_t* sourceBuffer, uint32_t sourceWidth, uint32_t sourceHeight,
	uint32_t components, uint8_t* targetBuffer, uint32_t targetWidth, uint32_t targetHeight)
{
	float xRatio = static_cast<float>(sourceWidth  - 1) / targetWidth;
	float yRatio = static_cast<float>(sourceHeight - 1) / targetHeight;

	uint32_t sourceRowStride = sourceWidth * components;
	uint32_t targetRowStride = targetWidth * components;

	uint32_t sourceXPixel, sourceYPixel, sourceIndex0, sourceIndex1, sourceIndex2, sourceIndex3;
	uint32_t targetIndex;
	uint8_t blue, green, red;
	float xWeight, yWeight;

	uint32_t sourceTaps[4];

	// Do a bilinear interpolation
	for (uint32_t y = 0; y < targetHeight; ++y)
	{
		for (uint32_t x = 0; x < targetWidth; ++x)
		{
			memset(sourceTaps, 0, sizeof(sourceTaps));
			sourceXPixel = static_cast<uint32_t>(xRatio * x);
			sourceYPixel = static_cast<uint32_t>(yRatio * y);
			xWeight = (xRatio * x) - sourceXPixel;
			yWeight = (yRatio * y) - sourceYPixel;

			sourceIndex0 = sourceYPixel * sourceRowStride + ( sourceXPixel      * components);
			sourceIndex1 = sourceYPixel * sourceRowStride + ((sourceXPixel + 1) * components);
			sourceIndex2 = (sourceYPixel + 1) * sourceRowStride + ( sourceXPixel      * components);
			sourceIndex3 = (sourceYPixel + 1) * sourceRowStride + ((sourceXPixel + 1) * components);

			sourceTaps[0] = *reinterpret_cast<const uint32_t*>(sourceBuffer + sourceIndex0);
			sourceTaps[1] = *reinterpret_cast<const uint32_t*>(sourceBuffer + sourceIndex1);
			sourceTaps[2] = *reinterpret_cast<const uint32_t*>(sourceBuffer + sourceIndex2);
			sourceTaps[3] = *reinterpret_cast<const uint32_t*>(sourceBuffer + sourceIndex3);

			red   = (sourceTaps[0] & 0xFF)*(1-xWeight)*(1-yWeight) + (sourceTaps[1] & 0xFF)*(xWeight)*(1-yWeight) +
	                (sourceTaps[2] & 0xFF)*(yWeight)*(1-xWeight)   + (sourceTaps[3] & 0xFF)*(xWeight*yWeight);
			green = (sourceTaps[0]>>8 & 0xFF)*(1-xWeight)*(1-yWeight) + (sourceTaps[1]>>8 & 0xFF)*(xWeight)*(1-yWeight) +
	                (sourceTaps[2]>>8 & 0xFF)*(yWeight)*(1-xWeight)   + (sourceTaps[3]>>8 & 0xFF)*(xWeight*yWeight);
			blue  = (sourceTaps[0]>>16 & 0xFF)*(1-xWeight)*(1-yWeight) + (sourceTaps[1]>>16 & 0xFF)*(xWeight)*(1-yWeight) +
	                (sourceTaps[2]>>16 & 0xFF)*(yWeight)*(1-xWeight)   + (sourceTaps[3]>>16 & 0xFF)*(xWeight*yWeight);

			targetIndex = y * targetRowStride + (x * components);
			targetBuffer[targetIndex]   = red;
			targetBuffer[targetIndex+1] = green;
			targetBuffer[targetIndex+2] = blue;
		}
	}

	return true;
}
} // lib
} // enlighten
