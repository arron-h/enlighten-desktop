#ifndef VALIDATION_H
#define VALIDATION_H

#include "logger.h"

#define VALIDATE_AND_RETURN(returnValue, expression, message, ...) \
	if(!(expression))\
	{\
		enlighten::lib::Logger::get().log(enlighten::lib::Logger::ERROR, "Validation failed! [%s:%d] " message, __FILE__, __LINE__, ##__VA_ARGS__);\
		return (returnValue);\
	}

#define VALIDATE(expression, message, ...) \
	if(!(expression))\
	{\
		enlighten::lib::Logger::get().log(enlighten::lib::Logger::ERROR, "Validation failed! [%s:%d] " message, __FILE__, __LINE__, ##__VA_ARGS__);\
		return false;\
	}

#define CHECK(expression) \
	if(!(expression))\
	{\
		return false;\
	}

#define CHECK_AND_RETURN(returnValue, expression) \
	if(!(expression))\
	{\
		return (returnValue);\
	}


#endif
