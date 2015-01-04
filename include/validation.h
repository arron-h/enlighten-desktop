#ifndef VALIDATION_H
#define VALIDATION_H

#define VALIDATE_AND_RETURN(retVal, expression, message, ...) \
	if(!expression) { printf("ERROR: Validation failed! [%s:%d] " message "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
	return retVal; }

#define VALIDATE(expression, message, ...) \
	if(!expression) { printf("ERROR: Validation failed! [%s:%d] " message "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
	return false; }

#endif