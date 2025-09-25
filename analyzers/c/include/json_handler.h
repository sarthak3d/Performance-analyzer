#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include "types.h"

// Parse JSON request to extract code
char* parse_json_request(const char* json_str);

// Convert analysis result to JSON string
char* result_to_json(ComplexityResult* result);

#endif // JSON_HANDLER_H