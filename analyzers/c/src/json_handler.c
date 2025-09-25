#include "json_handler.h"
#include "ast_visitor.h"
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  // Added this include

char* parse_json_request(const char* json_str) {
    if (!json_str) return NULL;
    
    cJSON* json = cJSON_Parse(json_str);
    if (!json) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "JSON parse error before: %s\n", error_ptr);
        }
        return NULL;
    }
    
    cJSON* code_item = cJSON_GetObjectItem(json, "code");
    if (!cJSON_IsString(code_item) || code_item->valuestring == NULL) {
        cJSON_Delete(json);
        return NULL;
    }
    
    char* code = strdup(code_item->valuestring);
    cJSON_Delete(json);
    
    return code;
}

char* result_to_json(ComplexityResult* result) {
    if (!result) return NULL;
    
    cJSON* json = cJSON_CreateObject();
    if (!json) return NULL;
    
    // Add basic complexity information
    cJSON_AddStringToObject(json, "timeComplexity", result->time_complexity);
    cJSON_AddStringToObject(json, "spaceComplexity", result->space_complexity);
    
    // Add detailed complexity cases
    cJSON_AddStringToObject(json, "bestCaseTime", result->best_case_time);
    cJSON_AddStringToObject(json, "averageCaseTime", result->average_case_time);
    cJSON_AddStringToObject(json, "worstCaseTime", result->worst_case_time);
    cJSON_AddStringToObject(json, "bestCaseSpace", result->best_case_space);
    cJSON_AddStringToObject(json, "averageCaseSpace", result->average_case_space);
    cJSON_AddStringToObject(json, "worstCaseSpace", result->worst_case_space);
    
    // Add analysis metadata
    cJSON_AddNumberToObject(json, "confidence", result->confidence);
    cJSON_AddStringToObject(json, "explanation", result->explanation);
    
    // Add patterns array
    cJSON* patterns_array = cJSON_CreateArray();
    if (result->patterns) {
        for (size_t i = 0; i < result->patterns->count; i++) {
            cJSON_AddItemToArray(patterns_array, cJSON_CreateString(result->patterns->items[i]));
        }
    }
    cJSON_AddItemToObject(json, "patterns", patterns_array);
    
    // Add suggestions array
    cJSON* suggestions_array = cJSON_CreateArray();
    if (result->suggestions) {
        for (size_t i = 0; i < result->suggestions->count; i++) {
            cJSON_AddItemToArray(suggestions_array, cJSON_CreateString(result->suggestions->items[i]));
        }
    }
    cJSON_AddItemToObject(json, "suggestions", suggestions_array);
    
    // Add code characteristics
    cJSON_AddBoolToObject(json, "recursive", result->recursive);
    cJSON_AddBoolToObject(json, "hasLoops", result->has_loops);
    cJSON_AddNumberToObject(json, "loopDepth", result->loop_depth);
    
    // Convert to string
    char* json_str = cJSON_Print(json);
    cJSON_Delete(json);
    
    return json_str;
}