#include "pattern_detector.hpp"
#include <algorithm>
#include <string>

std::vector<std::string> PatternDetector::detectPatterns(const std::string& code, 
                                                        const AnalysisData& astData) {
    std::vector<std::string> patterns;
    
    // Algorithm patterns using simple string search
    if (detectBubbleSort(code)) patterns.push_back("bubble_sort");
    if (detectQuickSort(code)) patterns.push_back("quick_sort");
    if (detectMergeSort(code)) patterns.push_back("merge_sort");
    if (detectBinarySearch(code)) patterns.push_back("binary_search");
    if (detectDynamicProgramming(code)) patterns.push_back("dynamic_programming");
    if (detectGraphAlgorithm(code)) patterns.push_back("graph_algorithm");
    
    // Structural patterns from AST
    if (astData.hasRecursion) patterns.push_back("recursion");
    if (astData.hasTemplates) patterns.push_back("templates");
    if (astData.hasDynamicMemory) patterns.push_back("dynamic_memory");
    
    // STL patterns
    if (!astData.stlAlgorithms.empty()) patterns.push_back("stl_algorithms");
    if (!astData.stlContainers.empty()) patterns.push_back("stl_containers");
    
    // Loop patterns
    if (astData.maxLoopDepth >= 2) patterns.push_back("nested_loops");
    
    return patterns;
}

bool PatternDetector::detectBubbleSort(const std::string& code) {
    // Simple string search instead of regex to avoid issues
    return (code.find("bubble") != std::string::npos && code.find("sort") != std::string::npos) ||
           (code.find("Bubble") != std::string::npos && code.find("Sort") != std::string::npos);
}

bool PatternDetector::detectQuickSort(const std::string& code) {
    return (code.find("quick") != std::string::npos && code.find("sort") != std::string::npos) ||
           (code.find("Quick") != std::string::npos && code.find("Sort") != std::string::npos) ||
           code.find("pivot") != std::string::npos ||
           code.find("partition") != std::string::npos;
}

bool PatternDetector::detectMergeSort(const std::string& code) {
    return (code.find("merge") != std::string::npos && code.find("sort") != std::string::npos) ||
           (code.find("Merge") != std::string::npos && code.find("Sort") != std::string::npos);
}

bool PatternDetector::detectBinarySearch(const std::string& code) {
    return (code.find("binary") != std::string::npos && code.find("search") != std::string::npos) ||
           (code.find("Binary") != std::string::npos && code.find("Search") != std::string::npos) ||
           (code.find("left") != std::string::npos && 
            code.find("right") != std::string::npos && 
            code.find("mid") != std::string::npos);
}

bool PatternDetector::detectDynamicProgramming(const std::string& code) {
    // Fixed: Simple string search without regex
    return code.find("dp[") != std::string::npos ||
           code.find("memo[") != std::string::npos ||
           code.find("cache[") != std::string::npos ||
           code.find("table[") != std::string::npos ||
           code.find("memoization") != std::string::npos;
}

bool PatternDetector::detectGraphAlgorithm(const std::string& code) {
    return code.find("dfs") != std::string::npos ||
           code.find("DFS") != std::string::npos ||
           code.find("bfs") != std::string::npos ||
           code.find("BFS") != std::string::npos ||
           code.find("dijkstra") != std::string::npos ||
           code.find("Dijkstra") != std::string::npos ||
           code.find("graph") != std::string::npos ||
           code.find("Graph") != std::string::npos;
}