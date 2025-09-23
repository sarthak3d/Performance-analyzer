#include <iostream>
#include <string>
#include <sstream>
#include "analyzer.hpp"
#include "json_handler.hpp"

int main(int argc, char* argv[]) {
    try {
        // Read JSON input from stdin
        std::string input;
        std::string line;
        while (std::getline(std::cin, line)) {
            input += line + "\n";
        }
        
        // Parse JSON input
        JsonHandler jsonHandler;
        auto request = jsonHandler.parseRequest(input);
        
        // Analyze code
        CppAnalyzer analyzer;
        auto result = analyzer.analyze(request.code);
        
        // Output result as JSON
        std::string jsonOutput = jsonHandler.resultToJson(result);
        std::cout << jsonOutput << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}