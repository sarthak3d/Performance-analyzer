"""
Bridge service to communicate with native language analyzers.
"""

import json
import subprocess
import os
from pathlib import Path
from typing import Dict, Any
from loguru import logger


class AnalyzerBridge:
    """Bridge to communicate with native language analyzers via subprocess."""
    
    def __init__(self):
        self.analyzer_commands = {
            'python': self._get_python_command,
            'java': ['java', '-jar', 'analyzers/java/target/java-analyzer-1.0.0-jar-with-dependencies.jar'],
            'cpp': ['./analyzers/cpp/build/cpp_analyzer'],
            'c': ['./analyzers/cpp/build/cpp_analyzer'],
            'javascript': ['node', 'analyzers/javascript/src/index.js'],
        }
    
    def analyze(self, code: str, language: str) -> Dict[str, Any]:
        """
        Analyze code using the appropriate native analyzer.
        
        Args:
            code: Source code to analyze
            language: Programming language
            
        Returns:
            Analysis result as dictionary
        """
        if language not in self.analyzer_commands:
            raise ValueError(f"Unsupported language: {language}")
        
        if language == 'python':
            # Python analyzer runs in-process
            return self._analyze_python(code)
        else:
            # Other analyzers run as subprocesses
            command = self.analyzer_commands[language]
            return self._run_analyzer(command, code, language)
    
    def _get_python_command(self):
        """Python runs in-process, not as subprocess."""
        pass
    
    def _analyze_python(self, code: str) -> Dict[str, Any]:
        """Run Python analyzer in-process."""
        from analyzers.python.analyzer import PythonComplexityAnalyzer
        
        analyzer = PythonComplexityAnalyzer()
        result = analyzer.analyze(code)
        
        # Convert to dict
        return {
            'timeComplexity': result.time_complexity.value,
            'spaceComplexity': result.space_complexity.value,
            'bestCaseTime': result.best_case_time.value,
            'averageCaseTime': result.average_case_time.value,
            'worstCaseTime': result.worst_case_time.value,
            'bestCaseSpace': result.best_case_space.value,
            'averageCaseSpace': result.average_case_space.value,
            'worstCaseSpace': result.worst_case_space.value,
            'confidence': result.confidence,
            'explanation': result.explanation,
            'patterns': result.patterns,
            'suggestions': result.suggestions,
            'recursive': result.recursive,
            'hasLoops': result.has_loops,
            'loopDepth': result.loop_depth
        }
    
    def _run_analyzer(self, command: list, code: str, language: str) -> Dict[str, Any]:
        """
        Run an external analyzer via subprocess.
        
        Args:
            command: Command to execute analyzer
            code: Source code to analyze
            language: Programming language
            
        Returns:
            Analysis result as dictionary
        """
        try:
            # Prepare input JSON
            input_data = json.dumps({
                'code': code,
                'language': language,
                'fileName': f'input.{self._get_extension(language)}'
            })
            
            # Run analyzer
            result = subprocess.run(
                command,
                input=input_data,
                capture_output=True,
                text=True,
                timeout=30,
                cwd=os.getcwd()
            )
            
            if result.returncode != 0:
                logger.error(f"Analyzer failed: {result.stderr}")
                raise RuntimeError(f"Analyzer failed: {result.stderr}")
            
            # Parse output JSON
            return json.loads(result.stdout)
            
        except subprocess.TimeoutExpired:
            logger.error("Analyzer timeout")
            raise RuntimeError("Analysis timeout")
        except json.JSONDecodeError as e:
            logger.error(f"Invalid JSON from analyzer: {e}")
            logger.error(f"Output was: {result.stdout}")
            raise RuntimeError("Invalid analyzer output")
        except Exception as e:
            logger.exception("Analyzer execution failed")
            raise RuntimeError(f"Analyzer failed: {str(e)}")
    
    def _get_extension(self, language: str) -> str:
        """Get file extension for language."""
        extensions = {
            'python': 'py',
            'java': 'java',
            'cpp': 'cpp',
            'c': 'c',
            'javascript': 'js'
        }
        return extensions.get(language, 'txt')