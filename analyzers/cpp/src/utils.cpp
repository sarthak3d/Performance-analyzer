#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace cpp_analyzer {

std::string Utils::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool Utils::contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

bool Utils::containsAny(const std::string& str, const std::vector<std::string>& substrs) {
    for (const auto& substr : substrs) {
        if (contains(str, substr)) {
            return true;
        }
    }
    return false;
}

int Utils::countOccurrences(const std::string& str, const std::string& substr) {
    int count = 0;
    size_t pos = 0;
    
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        ++count;
        pos += substr.length();
    }
    
    return count;
}

std::string Utils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

} // namespace cpp_analyzer