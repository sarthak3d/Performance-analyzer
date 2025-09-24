#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace cpp_analyzer {

class Utils {
public:
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static bool contains(const std::string& str, const std::string& substr);
    static bool containsAny(const std::string& str, const std::vector<std::string>& substrs);
    static int countOccurrences(const std::string& str, const std::string& substr);
    static std::string toLower(const std::string& str);
};

} // namespace cpp_analyzer

#endif // UTILS_HPP