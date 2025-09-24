"""
Bridge service to communicate with native language analyzers.
"""

import json
import subprocess
import os
import importlib
from pathlib import Path
from typing import Dict, Any, Optional
from loguru import logger


class AnalyzerBridge:
    """Bridge to communicate with native language analyzers via subprocess."""
    
    def __init__(self):
        self.config = self._load_config()
        self._validate_config()
    
    def _load_config(self) -> Dict[str, Any]:
        """Load analyzer configuration from JSON file."""
        config_path = Path(os.path.dirname(__file__)) / "../../config/analyzer_config.json"
        try:
            with open(config_path) as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            raise RuntimeError(f"Failed to load analyzer configuration: {e}")

    def _validate_config(self):
        """Validate the loaded configuration."""
        from ..utils.config_validator import ConfigValidator
        
        errors = ConfigValidator.validate_config(self.config)
        if errors:
            error_msg = "\n".join(errors)
            logger.error(f"Configuration validation failed:\n{error_msg}")
            raise ValueError(f"Invalid analyzer configuration:\n{error_msg}")

    def analyze(self, code: str, language: str) -> Dict[str, Any]:
        """
        Analyze code using the appropriate native analyzer.
        
        Args:
            code: Source code to analyze
            language: Programming language
            
        Returns:
            Analysis result as dictionary
        """
        if language not in self.config["analyzers"]:
            raise ValueError(f"Unsupported language: {language}")
            
        lang_config = self.config["analyzers"][language]
        if not lang_config.get("enabled", False):
            raise ValueError(f"Analyzer for {language} is disabled")
            
        if lang_config.get("in_process", False):
            # In-process analyzer (like Python)
            return self._analyze_in_process(code, language, lang_config)
        else:
            # External analyzer
            return self._run_analyzer(lang_config["command"], code, language)
    
    def _analyze_in_process(self, code: str, language: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Run analyzer in-process."""
        try:
            # Import the analyzer class dynamically
            module_path, class_name = config["analyzer_module"].rsplit('.', 1)
            module = importlib.import_module(module_path)
            analyzer_class = getattr(module, class_name)
            
            analyzer = analyzer_class()
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
        except Exception as e:
            logger.error(f"Failed to run in-process analyzer for {language}: {e}")
            raise RuntimeError(f"In-process analyzer failed: {str(e)}")
    
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
        """Get file extension for language from config."""
        if language not in self.config["analyzers"]:
            return 'txt'
        return self.config["analyzers"][language].get("file_extension", 'txt')