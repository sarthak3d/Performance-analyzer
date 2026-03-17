# Big O Analyzer

A professional-grade complexity analysis tool that supports multiple programming languages.

## Features

- Supports Python, Java, C, and JavaScript
- Both CLI and REST API interfaces
- Native language analyzers for accurate analysis
- Pattern recognition and complexity calculation
- Best/Average/Worst case analysis
- Confidence scoring

## Installation

```bash
# Build image
docker build -t performance-analyzer .

# Run in CLI mode
docker run -v "${PWD}:/app" performance-analyzer analyze script.py

# Run in API mode
docker run -p 5000:5000 -e API_MODE=true performance-analyzer
```

## Usage

### CLI Mode

```bash
#Display information about the analyzer
docker run -v "${PWD}:/app" performance-analyzer info

# Analyze a single file
docker run -v "${PWD}:/app" performance-analyzer analyze script.py

# Analyze with specific language and format
docker run -v "${PWD}:/app" performance-analyzer analyze Algorithm.java --language java --format json

# Batch analyze multiple files
docker run -v "${PWD}:/app" performance-analyzer batch ./src --pattern "*.py" --recursive
```

### API Mode

The analyzer can also run as a REST API server. To start in API mode:

```bash
# Using Docker
docker build -t performance-analyzer .
docker run -p 5000:5000 -e API_MODE=true performance-analyzer
```

#### API Endpoints

1. Analyze Single File
```http
POST /analyze
Content-Type: application/json

{
    "code": "def example(n):\\n    return n * 2",
    "language": "python",
    "filename": "example.py",
    "options": {
        "verbose": true
    }
}
```

2. Batch Analysis
```http
POST /batch
Content-Type: application/json

{
    "files": [
        {
            "code": "def example(n):\\n    return n * 2",
            "language": "python",
            "filename": "example.py"
        }
    ],
    "options": {
        "verbose": true
    }
}
```

3. Get Analyzer Info
```http
GET /info
```

## Environment Variables

- `API_MODE`: Set to "true" to run in API mode
- `API_PORT`: API server port (default: 5000)
- `API_HOST`: API server host (default: 0.0.0.0)
- `API_WORKERS`: Number of Gunicorn workers (default: 1)
- `API_DEBUG`: Enable debug mode in API (default: false)

## Configuration

Analyzer Configuration File Format
'''
{
  "#language_name": {
    "enabled": true,
    "timeout": 30,
    "max_file_size": 10485760,
    "file_extension": "ext",
    "in_process": false,
    "command": ["command", "arg1", "arg2"],
    // OR for in-process analyzers:
    "in_process": true,
    "analyzer_module": "pathto analyzer AnalyzerClass"
  }
}
'''
