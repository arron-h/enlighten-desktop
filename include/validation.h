#ifndef VALIDATION_H
#define VALIDATION_H

#include "logger.h"

#define VALIDATE_AND_RETURN(returnValue, expression, message, ...) \
	if(!(expression))\
	{\
		enlighten::lib::Logger::get().log(enlighten::lib::Logger::ERROR, "Validation failed! [%s:%d] " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
		return (returnValue);\
	}

#define VALIDATE(expression, message, ...) \
	if(!(expression))\
	{\
		enlighten::lib::Logger::get().log(enlighten::lib::Logger::ERROR, "Validation failed! [%s:%d] " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
		return false;\
	}

#endif
