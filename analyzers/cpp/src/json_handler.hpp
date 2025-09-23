#ifndef JSON_HANDLER_HPP
#define JSON_HANDLER_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "analyzer.hpp"

struct AnalysisRequest {
    std::string code;
    std::string language;
    std::string fileName;
};

class JsonHandler {
public:
    AnalysisRequest parseRequest(const std::string& jsonStr);
    std::string resultToJson(const ComplexityResult& result);
};

#endif // JSON_HANDLER_HPP