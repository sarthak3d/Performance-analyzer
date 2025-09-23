"""
CLI Interface for Big O Analyzer
Provides command-line interface with rich formatting and interactive features.
"""

import json
from pathlib import Path
from typing import List, Optional
from enum import Enum

import typer
from rich.console import Console
from rich.table import Table
from rich.panel import Panel
from rich.syntax import Syntax
from rich.progress import track
from loguru import logger

from .models.analysis import AnalysisRequest, AnalysisResult
from .services.orchestrator import Orchestrator
from .utils.formatters import format_complexity_table, format_json_output
from .utils.validators import validate_file_path, validate_language
from .utils.file_detector import detect_language

console = Console()
app = typer.Typer(
    name="bigo",
    help="Professional Big O Complexity Analyzer",
    add_completion=True,
)


class OutputFormat(str, Enum):
    """Output format options."""
    TABLE = "table"
    JSON = "json"
    DETAILED = "detailed"


class Language(str, Enum):
    """Supported programming languages."""
    PYTHON = "python"
    JAVA = "java"
    CPP = "cpp"
    C = "c"
    JAVASCRIPT = "javascript"
    AUTO = "auto"


@app.command()
def analyze(
    file_path: Path = typer.Argument(
        ...,
        help="Path to the code file to analyze",
        exists=True,
        file_okay=True,
        dir_okay=False,
        readable=True,
    ),
    language: Language = typer.Option(
        Language.AUTO,
        "--language", "-l",
        help="Programming language (auto-detect if not specified)",
    ),
    output_format: OutputFormat = typer.Option(
        OutputFormat.TABLE,
        "--format", "-f",
        help="Output format for analysis results",
    ),
    verbose: bool = typer.Option(
        False,
        "--verbose", "-v",
        help="Enable verbose output",
    ),
):
    """
    Analyze the complexity of a code file.
    
    Examples:
        bigo analyze script.py
        bigo analyze Algorithm.java --format json
        bigo analyze sort.cpp --language cpp --verbose
    """
    try:
        with console.status("[bold green]Analyzing code...", spinner="dots"):
            orchestrator = Orchestrator()
            
            # Validate inputs
            if not validate_file_path(file_path):
                console.print(f"[red]Error: Invalid file path: {file_path}[/red]")
                raise typer.Exit(1)
            
            # Auto-detect language if needed
            if language == Language.AUTO:
                detected_language = detect_language(file_path)
                if not detected_language:
                    console.print("[red]Error: Could not detect language[/red]")
                    raise typer.Exit(1)
                language_str = detected_language
            else:
                language_str = language.value  # Get string value from enum
            
            # Read file
            with open(file_path, 'r', encoding='utf-8') as f:
                code_content = f.read()
            
            # Create request
            request = AnalysisRequest(
                code=code_content,
                language=language_str,  # Use the string value
                file_name=file_path.name,
                options={"verbose": verbose}
            )
            
            # Perform analysis
            result = orchestrator.analyze(request)
            
            # Display results
            display_results(result, output_format, verbose)
            
    except FileNotFoundError:
        console.print(f"[red]Error: File not found: {file_path}[/red]")
        raise typer.Exit(1)
    except Exception as e:
        logger.exception("Analysis failed")
        console.print(f"[red]Error: {str(e)}[/red]")
        raise typer.Exit(1)


@app.command()
def batch(
    directory: Path = typer.Argument(
        ...,
        help="Directory containing code files",
        exists=True,
        dir_okay=True,
    ),
    pattern: str = typer.Option(
        "*",
        "--pattern", "-p",
        help="File pattern to match",
    ),
    recursive: bool = typer.Option(
        False,
        "--recursive", "-r",
        help="Recursively analyze files",
    ),
    output_file: Optional[Path] = typer.Option(
        None,
        "--output", "-o",
        help="Save results to file",
    ),
):
    """
    Batch analyze multiple files in a directory.
    
    Examples:
        bigo batch ./src --pattern "*.py" --recursive
        bigo batch ./algorithms --output results.json
    """
    orchestrator = Orchestrator()
    
    # Find files
    if recursive:
        files = list(directory.rglob(pattern))
    else:
        files = list(directory.glob(pattern))
    
    if not files:
        console.print(f"[yellow]No files found matching '{pattern}'[/yellow]")
        raise typer.Exit(0)
    
    console.print(f"[cyan]Found {len(files)} files[/cyan]")
    
    results = []
    for file_path in track(files, description="Analyzing..."):
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                code = f.read()
            
            language_str = detect_language(file_path)
            if not language_str:
                continue
            
            request = AnalysisRequest(
                code=code,
                language=language_str,
                file_name=file_path.name,
            )
            
            result = orchestrator.analyze(request)
            results.append(result)
            
        except Exception as e:
            logger.error(f"Failed to analyze {file_path}: {e}")
    
    # Display or save results
    if output_file:
        save_results(results, output_file)
    else:
        display_batch_results(results)


@app.command()
def info():
    """Display information about the analyzer."""
    info_text = """
[bold cyan]Big O Complexity Analyzer v1.0.0[/bold cyan]

[bold]Supported Languages:[/bold]
  • Python (*.py)
  • Java (*.java)
  • C/C++ (*.c, *.cpp)
  • JavaScript (*.js)

[bold]Complexity Classes:[/bold]
  • Time: O(1), O(log n), O(n), O(n log n), O(n²), O(n³), O(2^n), O(n!)
  • Space: O(1), O(log n), O(n), O(n²), O(2^n)

[bold]Features:[/bold]
  • Native language analyzers for accuracy
  • Pattern recognition
  • Best/Average/Worst case analysis
  • Confidence scoring
    """
    console.print(Panel(info_text, border_style="cyan"))


def display_results(result: AnalysisResult, format: OutputFormat, verbose: bool):
    """Display analysis results."""
    if format == OutputFormat.TABLE:
        table = format_complexity_table(result)
        console.print(table)
    elif format == OutputFormat.JSON:
        json_output = format_json_output(result)
        console.print_json(json.dumps(json_output))
    elif format == OutputFormat.DETAILED:
        display_detailed_results(result, verbose)


def display_detailed_results(result: AnalysisResult, verbose: bool):
    """Display detailed results with panel."""
    content = f"""
[bold]File:[/bold] {result.file_name}
[bold]Language:[/bold] {result.language}

[bold]Time Complexity:[/bold]
  • Best: {result.time_complexity.best_case}
  • Average: {result.time_complexity.average_case}
  • Worst: {result.time_complexity.worst_case}
  • Confidence: {result.time_complexity.confidence:.1%}

[bold]Space Complexity:[/bold]
  • Best: {result.space_complexity.best_case}
  • Average: {result.space_complexity.average_case}
  • Worst: {result.space_complexity.worst_case}
    """
    
    if result.explanation:
        content += f"\n[bold]Explanation:[/bold]\n{result.explanation}"
    
    if result.suggestions:
        content += "\n[bold]Suggestions:[/bold]"
        for suggestion in result.suggestions:
            content += f"\n  • {suggestion}"
    
    console.print(Panel(content, title="Analysis Results", border_style="green"))


def display_batch_results(results: List[AnalysisResult]):
    """Display batch analysis results."""
    table = Table(title="Batch Analysis Results")
    table.add_column("File", style="cyan")
    table.add_column("Time Complexity", justify="center")
    table.add_column("Space Complexity", justify="center")
    table.add_column("Confidence", justify="center")
    
    for result in results:
        table.add_row(
            result.file_name,
            result.time_complexity.worst_case,
            result.space_complexity.worst_case,
            f"{result.confidence:.0%}"
        )
    
    console.print(table)


def save_results(results: List[AnalysisResult], output_file: Path):
    """Save results to file."""
    data = [format_json_output(r) for r in results]
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)
    console.print(f"[green]Results saved to {output_file}[/green]")


def create_app() -> typer.Typer:
    """Create and return the Typer application."""
    return app


if __name__ == "__main__":
    app()