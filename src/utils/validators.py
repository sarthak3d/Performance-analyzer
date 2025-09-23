"""
Validation utilities for Big O Analyzer.
"""

from pathlib import Path
from typing import Optional


def validate_file_path(file_path: Path) -> bool:
    """
    Validate that a file path exists and is readable.
    
    Args:
        file_path: Path to validate
        
    Returns:
        True if valid, False otherwise
    """
    if not file_path.exists():
        return False
    if not file_path.is_file():
        return False
    if not file_path.stat().st_size > 0:
        return False
    return True


def validate_language(language: str) -> bool:
    """
    Validate that a language is supported.
    
    Args:
        language: Language to validate
        
    Returns:
        True if supported, False otherwise
    """
    supported = ['python', 'java', 'cpp', 'c', 'javascript', 'js']
    return language.lower() in supported