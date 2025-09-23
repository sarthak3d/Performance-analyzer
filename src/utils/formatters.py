"""
Output formatting utilities for Big O Analyzer.
"""

from typing import Dict, Any
from rich.table import Table
from rich.text import Text

from ..models.analysis import AnalysisResult


def format_complexity_table(result: AnalysisResult) -> Table:
    """
    Format analysis result as a Rich table.
    
    Args:
        result: Analysis result
        
    Returns:
        Formatted table
    """
    table = Table(title=f"Complexity Analysis: {result.file_name}")
    
    table.add_column("Metric", style="cyan", width=20)
    table.add_column("Best Case", justify="center")
    table.add_column("Average Case", justify="center")
    table.add_column("Worst Case", justify="center")
    table.add_column("Confidence", justify="center")
    
    # Add time complexity row
    table.add_row(
        "Time Complexity",
        Text(result.time_complexity.best_case, style="green"),
        Text(result.time_complexity.average_case, style="yellow"),
        Text(result.time_complexity.worst_case, style="red"),
        f"{result.time_complexity.confidence:.0%}"
    )
    
    # Add space complexity row
    table.add_row(
        "Space Complexity",
        Text(result.space_complexity.best_case, style="green"),
        Text(result.space_complexity.average_case, style="yellow"),
        Text(result.space_complexity.worst_case, style="red"),
        f"{result.space_complexity.confidence:.0%}"
    )
    
    return table


def format_json_output(result: AnalysisResult) -> Dict[str, Any]:
    """
    Format analysis result as JSON-serializable dictionary.
    
    Args:
        result: Analysis result
        
    Returns:
        Dictionary representation
    """
    return {
        "file_name": result.file_name,
        "language": result.language,
        "time_complexity": {
            "best_case": result.time_complexity.best_case,
            "average_case": result.time_complexity.average_case,
            "worst_case": result.time_complexity.worst_case,
            "confidence": result.time_complexity.confidence
        },
        "space_complexity": {
            "best_case": result.space_complexity.best_case,
            "average_case": result.space_complexity.average_case,
            "worst_case": result.space_complexity.worst_case,
            "confidence": result.space_complexity.confidence
        },
        "patterns": [
            {
                "name": p.name,
                "description": p.description,
                "impact": p.impact
            }
            for p in result.patterns_found
        ],
        "explanation": result.explanation,
        "suggestions": result.suggestions,
        "confidence": result.confidence,
        "analysis_time": result.analysis_time,
        "metadata": result.metadata
    }