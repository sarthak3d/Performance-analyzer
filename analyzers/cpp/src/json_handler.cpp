#include "json_handler.hpp"

using json = nlohmann::json;

AnalysisRequest JsonHandler::parseRequest(const std::string& jsonStr) {
    json j = json::parse(jsonStr);
    
    AnalysisRequest request;
    request.code = j["code"];
    request.language = j.value("language", "cpp");
    request.fileName = j.value("fileName", "input.cpp");
    
    return request;
}

std::string JsonHandler::resultToJson(const ComplexityResult& result) {
    json j;
    
    j["timeComplexity"] = result.timeComplexity;
    j["spaceComplexity"] = result.spaceComplexity;
    j["bestCaseTime"] = result.bestCaseTime;
    j["averageCaseTime"] = result.averageCaseTime;
    j["worstCaseTime"] = result.worstCaseTime;
    j["bestCaseSpace"] = result.bestCaseSpace;
    j["averageCaseSpace"] = result.averageCaseSpace;
    j["worstCaseSpace"] = result.worstCaseSpace;
    j["confidence"] = result.confidence;
    j["explanation"] = result.explanation;
    j["patterns"] = result.patterns;
    j["suggestions"] = result.suggestions;
    j["recursive"] = result.recursive;
    j["hasLoops"] = result.hasLoops;
    j["loopDepth"] = result.loopDepth;
    
    return j.dump(2);
}