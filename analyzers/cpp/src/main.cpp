#include <iostream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "analyzer.hpp"

using json = nlohmann::json;
using namespace cpp_analyzer;

struct Request {
    std::string code;
    std::string language;
    std::string fileName;
};

Request parseRequest(const std::string& jsonStr) {
    json j = json::parse(jsonStr);
    Request req;
    req.code = j["code"];
    req.language = j.value("language", "cpp");
    req.fileName = j.value("fileName", "input.cpp");
    return req;
}

std::string resultToJson(const AnalysisResult& result) {
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

int main() {
    try {
        std::string input;
        std::string line;
        while (std::getline(std::cin, line)) {
            input += line + "\n";
        }
        
        Request request = parseRequest(input);
        
        Analyzer analyzer;
        AnalysisResult result = analyzer.analyze(request.code);
        
        std::string output = resultToJson(result);
        std::cout << output << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        
        // Return a default error result
        json j;
        j["timeComplexity"] = "O(?)";
        j["spaceComplexity"] = "O(?)";
        j["confidence"] = 0.0;
        j["explanation"] = std::string("Error: ") + e.what();
        
        std::cout << j.dump(2) << std::endl;
        return 1;
    }
}