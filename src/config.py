"""
Configuration module for Big O Analyzer.
"""

import os
from pathlib import Path
from typing import Optional
from dataclasses import dataclass


@dataclass
class Settings:
    """Application settings."""
    
    # Logging
    log_level: str = "INFO"
    log_file: Optional[Path] = None
    
    # Analysis
    max_file_size: int = 10485760  # 10MB
    analysis_timeout: int = 30
    
    # Output
    default_format: str = "table"
    colors_enabled: bool = True
    
    # Paths
    knowledge_base_path: Path = Path("knowledge_base")
    
    def __init__(self):
        """Initialize settings from environment variables."""
        self.log_level = os.getenv("LOG_LEVEL", self.log_level)
        log_file = os.getenv("LOG_FILE")
        if log_file:
            self.log_file = Path(log_file)
        
        self.max_file_size = int(os.getenv("MAX_FILE_SIZE", str(self.max_file_size)))
        self.analysis_timeout = int(os.getenv("ANALYSIS_TIMEOUT", str(self.analysis_timeout)))
        
        self.default_format = os.getenv("DEFAULT_FORMAT", self.default_format)
        self.colors_enabled = os.getenv("COLORS_ENABLED", "true").lower() == "true"
        
        kb_path = os.getenv("KNOWLEDGE_BASE_PATH")
        if kb_path:
            self.knowledge_base_path = Path(kb_path)


def get_settings() -> Settings:
    """Get application settings."""
    return Settings()