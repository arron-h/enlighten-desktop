#include "previewsdatabase.h"
#include "previewentry.h"

#include "validation.h"

PreviewsDatabase::PreviewsDatabase()
{
}

PreviewsDatabase::~PreviewsDatabase()
{
}

bool PreviewsDatabase::initialiseWithFile(const char* fileName)
{
	VALIDATE(fileName, "fileName argument is invalid");

	FILE* file = fopen(fileName, "rb");
	VALIDATE(file, "Failed to open provided file: %s", fileName);

	// TODO - store or load sqlite database
	fclose(file);

	return true;
}

unsigned int PreviewsDatabase::numberOfPreviewEntries()
{
	return 0;
}

const PreviewEntry* PreviewsDatabase::entryForIndex(unsigned int index)
{
	return NULL;
}
