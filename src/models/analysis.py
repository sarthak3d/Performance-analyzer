"""
Analysis models for Big O Analyzer.
"""

from typing import List, Dict, Any, Optional
from pydantic import BaseModel


class AnalysisRequest(BaseModel):
    """Request model for code analysis."""
    code: str
    language: str
    file_name: str
    options: Dict[str, Any] = {}


class ComplexityInfo(BaseModel):
    """Complexity information for a specific aspect."""
    best_case: str
    average_case: str
    worst_case: str
    confidence: float


class Pattern(BaseModel):
    """Detected pattern in code."""
    name: str
    description: str
    impact: str


class AnalysisResult(BaseModel):
    """Result of complexity analysis."""
    file_name: str
    language: str
    time_complexity: ComplexityInfo
    space_complexity: ComplexityInfo
    patterns_found: List[Pattern] = []
    explanation: str = ""
    suggestions: List[str] = []
    confidence: float
    analysis_time: float = 0.0
    metadata: Dict[str, Any] = {}