"""
Configuration validator for analyzer settings.
"""

from typing import Dict, Any, List
from pathlib import Path
import json
from loguru import logger


class ConfigValidator:
    """Validates analyzer configuration files."""
    
    REQUIRED_ROOT_KEYS = ["version", "analyzers"]
    REQUIRED_ANALYZER_KEYS = ["enabled", "timeout", "max_file_size", "file_extension"]
    
    @staticmethod
    def validate_config(config: Dict[str, Any]) -> List[str]:
        """
        Validate the analyzer configuration.
        
        Args:
            config: Configuration dictionary to validate
            
        Returns:
            List of validation errors (empty if valid)
        """
        errors = []
        
        # Check root structure
        for key in ConfigValidator.REQUIRED_ROOT_KEYS:
            if key not in config:
                errors.append(f"Missing required root key: {key}")
                
        if "analyzers" not in config:
            return errors
            
        # Validate each analyzer configuration
        for lang, analyzer_config in config["analyzers"].items():
            if not isinstance(analyzer_config, dict):
                errors.append(f"Invalid analyzer configuration for {lang}: must be a dictionary")
                continue
                
            # Check required fields
            for key in ConfigValidator.REQUIRED_ANALYZER_KEYS:
                if key not in analyzer_config:
                    errors.append(f"Missing required key '{key}' for analyzer '{lang}'")
            
            # Validate enabled flag
            if "enabled" in analyzer_config and not isinstance(analyzer_config["enabled"], bool):
                errors.append(f"Invalid 'enabled' value for {lang}: must be boolean")
            
            # Validate timeout
            if "timeout" in analyzer_config:
                try:
                    timeout = float(analyzer_config["timeout"])
                    if timeout <= 0:
                        errors.append(f"Invalid timeout for {lang}: must be positive")
                except (ValueError, TypeError):
                    errors.append(f"Invalid timeout for {lang}: must be a number")
            
            # Validate max file size
            if "max_file_size" in analyzer_config:
                try:
                    size = int(analyzer_config["max_file_size"])
                    if size <= 0:
                        errors.append(f"Invalid max_file_size for {lang}: must be positive")
                except (ValueError, TypeError):
                    errors.append(f"Invalid max_file_size for {lang}: must be an integer")
            
            # Validate analyzer specific settings
            if analyzer_config.get("in_process", False):
                if not analyzer_config.get("analyzer_module"):
                    errors.append(f"Missing analyzer_module for in-process analyzer {lang}")
            else:
                if not analyzer_config.get("command"):
                    errors.append(f"Missing command for external analyzer {lang}")
                elif not isinstance(analyzer_config["command"], list):
                    errors.append(f"Invalid command for {lang}: must be a list")
                    
        return errors
    
    @staticmethod
    def validate_config_file(config_path: Path) -> List[str]:
        """
        Validate a configuration file.
        
        Args:
            config_path: Path to the configuration file
            
        Returns:
            List of validation errors (empty if valid)
        """
        try:
            with open(config_path) as f:
                config = json.load(f)
            return ConfigValidator.validate_config(config)
        except json.JSONDecodeError as e:
            return [f"Invalid JSON in configuration file: {str(e)}"]
        except Exception as e:
            return [f"Error reading configuration file: {str(e)}"]