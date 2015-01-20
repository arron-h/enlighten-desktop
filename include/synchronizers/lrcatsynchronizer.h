#ifndef LRCATSYNCHRONIZER_H
#define LRCATSYNCHRONIZER_H

#include "abstractsynchronizer.h"

namespace enlighten
{
namespace lib
{
class LrCatSynchronizer : public AbstractSynchronizer
{
public:
	LrCatSynchronizer();
	~LrCatSynchronizer();

	bool beginSynchronizingFile(const std::string& file, const AWSConfig& awsConfig);
	bool stopSynchronizingFile();
};
} // lib
} // enlighten

#endif
