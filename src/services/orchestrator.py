"""
Orchestrator service using native language analyzers.
"""

from typing import Optional
from pathlib import Path
from loguru import logger
import time

from ..models.analysis import AnalysisRequest, AnalysisResult, ComplexityInfo, Pattern
from .analyzer_bridge import AnalyzerBridge


class Orchestrator:
    """Orchestrates complexity analysis using native language analyzers."""
    
    def __init__(self, knowledge_base_path: Optional[Path] = None):
        """Initialize the orchestrator with analyzer bridge."""
        self.bridge = AnalyzerBridge()
        self.knowledge_base_path = knowledge_base_path or Path("knowledge_base")
        logger.info("Orchestrator initialized with native language analyzers")
    
    def analyze(self, request: AnalysisRequest) -> AnalysisResult:
        """
        Analyze code using native language analyzer.
        
        Args:
            request: Analysis request containing code and options
            
        Returns:
            AnalysisResult with complexity information
        """
        try:
            start_time = time.time()
            language = request.language.lower()
            
            # Validate language against configuration
            if language not in self.bridge.config["analyzers"]:
                raise ValueError(f"Unsupported language: {language}")
                
            lang_config = self.bridge.config["analyzers"][language]
            if not lang_config.get("enabled", False):
                raise ValueError(f"Analyzer for {language} is currently disabled")
            
            logger.info(f"Analyzing {language} code from {request.file_name}")
            
            # Call native analyzer through bridge
            result_dict = self.bridge.analyze(request.code, language)
            
            # Calculate analysis time
            analysis_time = time.time() - start_time
            
            # Convert to API model
            result = self._convert_to_analysis_result(result_dict, request)
            result.analysis_time = analysis_time
            
            return result
            
        except Exception as e:
            logger.exception(f"Analysis failed for {request.file_name}")
            raise RuntimeError(f"Analysis failed: {str(e)}")
    
    def _convert_to_analysis_result(
        self, 
        result_dict: dict, 
        request: AnalysisRequest
    ) -> AnalysisResult:
        """Convert native analyzer result to API model."""
        # Create time complexity info
        time_complexity = ComplexityInfo(
            best_case=result_dict.get('bestCaseTime', result_dict.get('timeComplexity', 'O(?)')),
            average_case=result_dict.get('averageCaseTime', result_dict.get('timeComplexity', 'O(?)')),
            worst_case=result_dict.get('worstCaseTime', result_dict.get('timeComplexity', 'O(?)')),
            confidence=result_dict.get('confidence', 0.0)
        )
        
        # Create space complexity info
        space_complexity = ComplexityInfo(
            best_case=result_dict.get('bestCaseSpace', result_dict.get('spaceComplexity', 'O(?)')),
            average_case=result_dict.get('averageCaseSpace', result_dict.get('spaceComplexity', 'O(?)')),
            worst_case=result_dict.get('worstCaseSpace', result_dict.get('spaceComplexity', 'O(?)')),
            confidence=result_dict.get('confidence', 0.0)
        )
        
        # Convert patterns
        patterns = [
            Pattern(
                name=pattern,
                description=self._get_pattern_description(pattern),
                impact="Affects complexity calculation"
            )
            for pattern in result_dict.get('patterns', [])
        ]
        
        # Create analysis result
        return AnalysisResult(
            file_name=request.file_name,
            language=request.language,
            time_complexity=time_complexity,
            space_complexity=space_complexity,
            patterns_found=patterns,
            explanation=result_dict.get('explanation', ''),
            suggestions=result_dict.get('suggestions', []),
            confidence=result_dict.get('confidence', 0.0),
            analysis_time=0.0,
            metadata={
                'recursive': result_dict.get('recursive', False),
                'hasLoops': result_dict.get('hasLoops', False),
                'loopDepth': result_dict.get('loopDepth', 0),
            }
        )
    
    def _get_pattern_description(self, pattern: str) -> str:
        """Get description for a pattern."""
        descriptions = {
            'recursion': 'Recursive function calls detected',
            'sorting': 'Sorting algorithm implementation',
            'nested_loops': 'Nested loop structures',
            'dynamic_programming': 'Dynamic programming pattern',
            'binary_search': 'Binary search algorithm',
            'bubble_sort': 'Bubble sort algorithm',
            'quick_sort': 'Quick sort algorithm',
            'merge_sort': 'Merge sort algorithm',
            'async_await': 'Asynchronous programming pattern',
            'dom_manipulation': 'DOM manipulation operations',
            'method_chaining': 'Method chaining pattern',
            'stream_api': 'Java Stream API usage',
            'stl_algorithms': 'STL algorithm usage',
            'templates': 'Template metaprogramming',
        }
        return descriptions.get(pattern, f"Pattern: {pattern}")