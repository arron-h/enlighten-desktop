#ifndef LRPREV_H
#define LRPREV_H

#include <cstdio>

class LrPrev
{
public:
	LrPrev();
	~LrPrev();

	bool initialiseWithFile(const char* fileName);
	unsigned char* extractFromLevel(int level, unsigned int& numBytes);

private:
	bool skipNullBytes(int numberOfBytesToSkip);
	unsigned int swapEndian4Bytes(unsigned int value);

	bool readHeader();
	bool readJsonBlob();

	FILE*		_fileHandle;
};

#endif
