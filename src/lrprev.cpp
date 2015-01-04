#include "lrprev.h"
#include <string>
#include <stdlib.h>

#include "validation.h"

static unsigned int swapEndian4(unsigned int val)
{
	unsigned int v = 0;
	unsigned char* sourceBytes = reinterpret_cast<unsigned char*>(&val);
	unsigned char* targetBytes = reinterpret_cast<unsigned char*>(&v);
	
	targetBytes[0] = sourceBytes[3];
	targetBytes[1] = sourceBytes[2];
	targetBytes[2] = sourceBytes[1];
	targetBytes[3] = sourceBytes[0];
	
	return v;
}

LrPrev::LrPrev() : _fileHandle(NULL)
{
}

LrPrev::~LrPrev()
{
	fclose(_fileHandle);
	_fileHandle = NULL;
}

bool LrPrev::initialiseWithFile(const char* fileName)
{
	VALIDATE(fileName, "fileName argument is invalid");
	
	FILE* file = fopen(fileName, "rb");
	VALIDATE(file, "Failed to open provided file: %s", fileName);
	
	char markerBytes[4];
	fread(markerBytes, 1, 4, file);
	
	VALIDATE((strncmp(markerBytes, "AgHg", 4) == 0), "Marker bytes are invalid");
	
	_fileHandle = file;
	return true;
}

unsigned char* LrPrev::extractFromLevel(int level, unsigned int& jpegByteCount)
{
	VALIDATE_AND_RETURN(NULL, _fileHandle, "File handle is not valid!");
	
	// Make sure the file handle points to the begining of the file.
	rewind(_fileHandle);
	
	unsigned char* jpegBytes = NULL;
	
	// Seek past the first 4 marker bytes, and 4 further bytes. Probably a tag or something
	fseek(_fileHandle, 8, SEEK_CUR);
	
	VALIDATE_AND_RETURN(NULL, skipNullBytes(4), "Bytes skipped were not null!");
	
	unsigned int jsonSize = 0;
	fread(&jsonSize, 4, 1, _fileHandle);
	jsonSize = swapEndian4(jsonSize);
	
	VALIDATE_AND_RETURN(NULL, skipNullBytes(4), "Bytes skipped were not null!");
	
	unsigned int jsonEndPadding = 0;
	fread(&jsonEndPadding, 4, 1, _fileHandle);
	jsonEndPadding = swapEndian4(jsonEndPadding);
	
	// Seek past 'header  '.
	fseek(_fileHandle, 8, SEEK_CUR);
	
	// Now seek past the json body. We're not interested in it (maybe in the future)
	fseek(_fileHandle, jsonSize, SEEK_CUR);
	
	// Seek past the padding
	fseek(_fileHandle, jsonEndPadding, SEEK_CUR);
	
	// Loop all the levels
	while (!feof(_fileHandle))
	{
		// Seek past the marker, and 4 further bytes. Probably a tag or something.
		fseek(_fileHandle, 8, SEEK_CUR);
		
		VALIDATE_AND_RETURN(NULL, skipNullBytes(4), "Bytes skipped were not null!");
		
		unsigned int levelSize = 0;
		fread(&levelSize, 4, 1, _fileHandle);
		levelSize = swapEndian4(levelSize);
		
		VALIDATE_AND_RETURN(NULL, skipNullBytes(4), "Bytes skipped were not null!");
		
		unsigned int endPaddingBytes = 0;
		fread(&endPaddingBytes, 4, 1, _fileHandle);
		endPaddingBytes = swapEndian4(endPaddingBytes);
		
		// Read the level descriptor
		char levelDescriptor[32];
		for (int strIdx = 0;;++strIdx)
		{
			fread(levelDescriptor+strIdx, 1, 1, _fileHandle);
			
			if (*(levelDescriptor+strIdx) == '\0')
				break;
		}
		
		// Determine the level
		int levelNumber = 0;
		sscanf(levelDescriptor, "level_%d", &levelNumber);
		
		VALIDATE_AND_RETURN(NULL, (levelNumber!=0), "Level number could not be parsed");
		
		if (levelNumber == level)
		{
			// Now read the jpeg data block
			jpegBytes = (unsigned char*)malloc(levelSize);
			VALIDATE_AND_RETURN(NULL, jpegBytes, "Could not allocate memory for Jpeg data");
			
			fread(jpegBytes, 1,  levelSize, _fileHandle);
			jpegByteCount = levelSize;
			break;
		}
		else
		{
			fseek(_fileHandle, levelSize, SEEK_CUR);
		}
		
		// Now skip the padding bytes
		fseek(_fileHandle, endPaddingBytes, SEEK_CUR);
	}
	
	return jpegBytes;
}

bool LrPrev::skipNullBytes(int numberOfBytesToSkip)
{
	int testBytes = 0;
	fread(&testBytes, 4, 1, _fileHandle);
	
	return testBytes == 0;
}
