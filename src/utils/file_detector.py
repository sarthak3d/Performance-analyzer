"""
File type detection utilities for Big O Analyzer.
"""

from pathlib import Path
from typing import Optional


def detect_language(file_path: Path) -> Optional[str]:
    """
    Detect programming language from file extension.
    
    Args:
        file_path: Path to file
        
    Returns:
        Language name or None if not detected
    """
    extension_map = {
        '.py': 'python',
        '.java': 'java',
        '.cpp': 'cpp',
        '.cc': 'cpp',
        '.cxx': 'cpp',
        '.c': 'c',
        '.h': 'c',
        '.hpp': 'cpp',
        '.js': 'javascript',
        '.jsx': 'javascript',
        '.ts': 'javascript',
        '.tsx': 'javascript',
    }
    
    suffix = file_path.suffix.lower()
    return extension_map.get(suffix)