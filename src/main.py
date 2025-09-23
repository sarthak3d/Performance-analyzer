#!/usr/bin/env python3
"""
Big O Analyzer - Main Entry Point
Professional-grade complexity analysis tool for multiple programming languages.
"""

import sys
from pathlib import Path

import typer
from rich.console import Console
from loguru import logger

from .cli import create_app
from .config import Settings
from .utils.logger import setup_logging

console = Console()


def main():
    """Main entry point for the Big O Analyzer."""
    try:
        # Setup logging
        settings = Settings()
        setup_logging(settings.log_level, settings.log_file)
        
        # Create and run CLI app
        app = create_app()
        app()
        
    except KeyboardInterrupt:
        console.print("\n[yellow]Analysis interrupted by user[/yellow]")
        sys.exit(0)
    except Exception as e:
        logger.exception("Unexpected error occurred")
        console.print(f"[red]Error: {str(e)}[/red]")
        sys.exit(1)


if __name__ == "__main__":
    main()