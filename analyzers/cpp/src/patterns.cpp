#include "patterns.hpp"
#include "utils.hpp"
#include <regex>
#include <algorithm>
#include <stack>

namespace cpp_analyzer {

PatternDetector::PatternDetector() {}

CodeFeatures PatternDetector::detectFeatures(const std::string& code) {
    CodeFeatures features;
    
    // Clean the code
    std::string cleanCode = removeComments(code);
    cleanCode = removeStrings(cleanCode);
    
    // Detect loops
    features.maxNestedLoops = countLoopDepth(cleanCode);
    features.loopDepth = features.maxNestedLoops;
    features.hasLogarithmicLoop = hasLogarithmicPattern(cleanCode);
    
    // Detect recursion
    features.hasRecursion = detectRecursion(cleanCode);
    if (features.hasRecursion) {
        auto functions = extractFunctionNames(cleanCode);
        for (const auto& func : functions) {
            int calls = countRecursiveCalls(cleanCode, func);
            if (calls >= 2) {
                features.hasMultipleRecursiveCalls = true;
                features.recursiveCallCount = std::max(features.recursiveCallCount, calls);
                features.recursiveFunctions.push_back(func);
            } else if (calls == 1) {
                features.recursiveFunctions.push_back(func);
            }
        }
    }
    
    // Detect algorithms
    features.hasSorting = detectSorting(cleanCode);
    features.hasBinarySearch = detectBinarySearch(cleanCode);
    features.hasDynamicProgramming = detectDynamicProgramming(cleanCode);
    
    // Detect STL
    features.hasSTLSort = detectSTLSort(cleanCode);
    features.hasSTLContainers = detectSTLContainers(cleanCode);
    
    // Populate detected patterns
    if (features.hasLogarithmicLoop) features.detectedPatterns.push_back("logarithmic_loop");
    if (features.hasRecursion) features.detectedPatterns.push_back("recursion");
    if (features.hasMultipleRecursiveCalls) features.detectedPatterns.push_back("exponential_recursion");
    if (features.hasSorting) features.detectedPatterns.push_back("sorting");
    if (features.hasBinarySearch) features.detectedPatterns.push_back("binary_search");
    if (features.hasDynamicProgramming) features.detectedPatterns.push_back("dynamic_programming");
    if (features.hasSTLSort) features.detectedPatterns.push_back("stl_sort");
    
    return features;
}

int PatternDetector::countLoopDepth(const std::string& code) {
    int maxDepth = 0;
    int currentDepth = 0;
    std::stack<char> braces;
    bool inLoop = false;
    
    // Keywords that indicate loops
    std::vector<std::string> loopKeywords = {"for", "while", "do"};
    
    for (size_t i = 0; i < code.length(); ++i) {
        // Check for loop keywords
        for (const auto& keyword : loopKeywords) {
            if (i + keyword.length() < code.length()) {
                std::string substr = code.substr(i, keyword.length());
                if (substr == keyword) {
                    // Check if it's actually a keyword (not part of another word)
                    bool isKeyword = true;
                    if (i > 0 && std::isalnum(code[i-1])) isKeyword = false;
                    if (i + keyword.length() < code.length() && std::isalnum(code[i + keyword.length()])) isKeyword = false;
                    
                    if (isKeyword) {
                        inLoop = true;
                        i += keyword.length() - 1;
                        break;
                    }
                }
            }
        }
        
        // Track braces for loop bodies
        if (inLoop && code[i] == '{') {
            currentDepth++;
            maxDepth = std::max(maxDepth, currentDepth);
            inLoop = false;
        } else if (code[i] == '}' && currentDepth > 0) {
            currentDepth--;
        }
    }
    
    return maxDepth;
}

bool PatternDetector::hasLogarithmicPattern(const std::string& code) {
    // Look for patterns like i *= 2, i /= 2, i <<= 1, i >>= 1
    std::vector<std::string> patterns = {
        "*=", "/=", "<<=", ">>="
    };
    
    for (const auto& pattern : patterns) {
        if (code.find(pattern) != std::string::npos) {
            // Simple heuristic: if we see these operators, likely logarithmic
            // More sophisticated analysis would check if it's in a loop condition
            return true;
        }
    }
    
    // Also check for patterns like i = i * 2, i = i / 2
    std::regex mulDivPattern(R"(\w+\s*=\s*\w+\s*[*/]\s*\d+)");
    if (std::regex_search(code, mulDivPattern)) {
        return true;
    }
    
    return false;
}

bool PatternDetector::detectRecursion(const std::string& code) {
    auto functions = extractFunctionNames(code);
    
    for (const auto& func : functions) {
        // Look for function calling itself
        std::string pattern = func + "\\s*\KATEX_INLINE_OPEN";
        std::regex funcCallRegex(pattern);
        
        // Find the function definition
        size_t funcPos = code.find(func);
        if (funcPos != std::string::npos) {
            // Look for calls within the function body
            std::string remaining = code.substr(funcPos + func.length());
            if (std::regex_search(remaining, funcCallRegex)) {
                return true;
            }
        }
    }
    
    return false;
}

int PatternDetector::countRecursiveCalls(const std::string& code, const std::string& functionName) {
    int count = 0;
    std::string pattern = functionName + "\\s*\KATEX_INLINE_OPEN";
    std::regex funcCallRegex(pattern);
    
    // Find function definition
    size_t funcPos = code.find(functionName);
    if (funcPos != std::string::npos) {
        // Look for return statements with multiple recursive calls
        size_t returnPos = code.find("return", funcPos);
        if (returnPos != std::string::npos) {
            size_t endPos = code.find(";", returnPos);
            if (endPos != std::string::npos) {
                std::string returnStmt = code.substr(returnPos, endPos - returnPos);
                
                auto begin = std::sregex_iterator(returnStmt.begin(), returnStmt.end(), funcCallRegex);
                auto end = std::sregex_iterator();
                count = std::distance(begin, end);
            }
        }
    }
    
    return count;
}

bool PatternDetector::detectSorting(const std::string& code) {
    std::vector<std::string> sortPatterns = {
        "bubble", "quick", "merge", "heap", "insertion", "selection"
    };
    
    for (const auto& pattern : sortPatterns) {
        if (Utils::contains(Utils::toLower(code), pattern + "sort") || 
            Utils::contains(Utils::toLower(code), pattern + "_sort")) {
            return true;
        }
    }
    
    // Check for swap operations in nested loops (common in sorting)
    if (code.find("swap") != std::string::npos && countLoopDepth(code) >= 2) {
        return true;
    }
    
    return false;
}

bool PatternDetector::detectBinarySearch(const std::string& code) {
    // Check for binary search patterns
    bool hasLeft = code.find("left") != std::string::npos || code.find("low") != std::string::npos;
    bool hasRight = code.find("right") != std::string::npos || code.find("high") != std::string::npos;
    bool hasMid = code.find("mid") != std::string::npos || code.find("middle") != std::string::npos;
    
    if (hasLeft && hasRight && hasMid) {
        // Check for the characteristic division by 2
        if (code.find("/ 2") != std::string::npos || code.find("/2") != std::string::npos ||
            code.find(">> 1") != std::string::npos || code.find(">>1") != std::string::npos) {
            return true;
        }
    }
    
    return Utils::contains(Utils::toLower(code), "binary_search") || 
           Utils::contains(Utils::toLower(code), "binarysearch");
}

bool PatternDetector::detectDynamicProgramming(const std::string& code) {
    // Look for DP table patterns
    std::vector<std::string> dpPatterns = {
        "dp[", "memo[", "cache[", "table[", "dp_", "memo_", "memoization"
    };
    
    return Utils::containsAny(code, dpPatterns);
}

bool PatternDetector::detectSTLSort(const std::string& code) {
    // Check for std::sort and related functions
    std::vector<std::string> stlSortPatterns = {
        "std::sort", "std::stable_sort", "std::partial_sort",
        "sort(", "stable_sort(", "partial_sort("
    };
    
    return Utils::containsAny(code, stlSortPatterns);
}

bool PatternDetector::detectSTLContainers(const std::string& code) {
    std::vector<std::string> containers = {
        "std::vector", "std::list", "std::deque", "std::set", "std::map",
        "std::unordered_set", "std::unordered_map", "std::array",
        "vector<", "list<", "deque<", "set<", "map<", "array<"
    };
    
    return Utils::containsAny(code, containers);
}

std::vector<std::string> PatternDetector::extractFunctionNames(const std::string& code) {
    std::vector<std::string> functions;
    
    // Simple regex to find function definitions
    std::regex funcRegex(R"(\b(\w+)\s*KATEX_INLINE_OPEN[^)]*KATEX_INLINE_CLOSE\s*\{)");
    
    auto begin = std::sregex_iterator(code.begin(), code.end(), funcRegex);
    auto end = std::sregex_iterator();
    
    for (auto it = begin; it != end; ++it) {
        std::string funcName = (*it)[1];
        // Skip common keywords
        if (funcName != "if" && funcName != "for" && funcName != "while" && 
            funcName != "switch" && funcName != "catch") {
            functions.push_back(funcName);
        }
    }
    
    return functions;
}

std::string PatternDetector::removeComments(const std::string& code) {
    std::string result;
    bool inLineComment = false;
    bool inBlockComment = false;
    
    for (size_t i = 0; i < code.length(); ++i) {
        if (!inLineComment && !inBlockComment) {
            if (i + 1 < code.length()) {
                if (code[i] == '/' && code[i + 1] == '/') {
                    inLineComment = true;
                    ++i;
                } else if (code[i] == '/' && code[i + 1] == '*') {
                    inBlockComment = true;
                    ++i;
                } else {
                    result += code[i];
                }
            } else {
                result += code[i];
            }
        } else if (inLineComment) {
            if (code[i] == '\n') {
                inLineComment = false;
                result += '\n';
            }
        } else if (inBlockComment) {
            if (i + 1 < code.length() && code[i] == '*' && code[i + 1] == '/') {
                inBlockComment = false;
                ++i;
            }
        }
    }
    
    return result;
}

std::string PatternDetector::removeStrings(const std::string& code) {
    std::string result;
    bool inString = false;
    bool inChar = false;
    bool escaped = false;
    
    for (size_t i = 0; i < code.length(); ++i) {
        if (!escaped && !inChar && code[i] == '"') {
            inString = !inString;
            result += '"';
        } else if (!escaped && !inString && code[i] == '\'') {
            inChar = !inChar;
            result += '\'';
        } else if (!inString && !inChar) {
            result += code[i];
        } else if (inString) {
            result += ' '; // Replace string content with space
        } else if (inChar) {
            result += ' '; // Replace char content with space
        }
        
        escaped = (code[i] == '\\' && !escaped);
    }
    
    return result;
}

} // namespace cpp_analyzer