"""
REST API Interface for Big O Analyzer
Provides HTTP endpoints for complexity analysis.
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
from pathlib import Path
import tempfile
import os
from loguru import logger

from .models.analysis import AnalysisRequest, AnalysisResult
from .services.orchestrator import Orchestrator
from .utils.formatters import format_json_output
from .utils.file_detector import detect_language

# Create Flask app
app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Configure production logging
if os.getenv('API_MODE') == 'true':
    logger.info("Initializing API in production mode")

# Initialize orchestrator (will be created per-worker in Gunicorn)
orchestrator = None

def get_orchestrator():
    """Lazy initialization of orchestrator (one per worker)."""
    global orchestrator
    if orchestrator is None:
        logger.info("Initializing Orchestrator for worker")
        orchestrator = Orchestrator()
    return orchestrator


@app.route("/", methods=["GET"])
def index():
    """API root endpoint."""
    return jsonify({
        "name": "Big O Complexity Analyzer API",
        "version": "1.0.0",
        "status": "running",
        "endpoints": {
            "analyze": "/analyze (POST)",
            "batch": "/batch (POST)",
            "info": "/info (GET)",
            "health": "/health (GET)"
        }
    })


@app.route("/health", methods=["GET"])
def health():
    """Health check endpoint."""
    return jsonify({
        "status": "healthy",
        "service": "performance-analyzer"
    }), 200


@app.route("/analyze", methods=["POST"])
def analyze():
    """
    Analyze code complexity via HTTP POST request.
    
    Expected JSON body:
    {
        "code": "string",          # Required: Source code to analyze
        "language": "string",      # Optional: Programming language (auto-detect if not provided)
        "filename": "string",      # Optional: Original filename
        "options": {              # Optional: Additional options
            "verbose": boolean
        }
    }
    
    Returns:
        JSON object containing analysis results
    """
    try:
        # Get request data
        data = request.get_json()
        
        if not data or "code" not in data:
            return jsonify({"error": "Missing required field: code"}), 400
        
        code = data["code"]
        language = data.get("language")
        filename = data.get("filename", "unnamed_file")
        options = data.get("options", {})
        
        # Auto-detect language if not provided
        if not language:
            # Create temporary file to detect language
            with tempfile.NamedTemporaryFile(mode='w', suffix=f"_{filename}", delete=False) as temp_file:
                temp_file.write(code)
                temp_file_path = temp_file.name
            
            try:
                language = detect_language(Path(temp_file_path))
                if not language:
                    return jsonify({"error": "Could not detect language. Please specify language explicitly."}), 400
            finally:
                # Clean up temp file
                try:
                    os.unlink(temp_file_path)
                except:
                    pass
        
        # Create analysis request
        request_obj = AnalysisRequest(
            code=code,
            language=language,
            file_name=filename,
            options=options
        )
        
        # Perform analysis
        orch = get_orchestrator()
        result = orch.analyze(request_obj)
        
        # Format and return results
        return jsonify(format_json_output(result)), 200
    
    except ValueError as e:
        logger.error(f"Validation error: {e}")
        return jsonify({"error": str(e)}), 400
    except Exception as e:
        logger.exception("Analysis failed")
        return jsonify({"error": "Internal server error", "message": str(e)}), 500


@app.route("/batch", methods=["POST"])
def batch_analyze():
    """
    Analyze multiple code files in a single request.
    
    Expected JSON body:
    {
        "files": [
            {
                "code": "string",          # Required: Source code
                "language": "string",      # Optional: Programming language
                "filename": "string"       # Optional: Original filename
            }
        ],
        "options": {                      # Optional: Analysis options
            "verbose": boolean
        }
    }
    """
    try:
        data = request.get_json()
        
        if not data or "files" not in data or not isinstance(data["files"], list):
            return jsonify({"error": "Missing or invalid files array"}), 400
        
        if len(data["files"]) > 50:
            return jsonify({"error": "Maximum 50 files per batch request"}), 400
        
        options = data.get("options", {})
        results = []
        errors = []
        
        orch = get_orchestrator()
        
        for idx, file_data in enumerate(data["files"]):
            if "code" not in file_data:
                errors.append({
                    "index": idx,
                    "error": "Missing code field"
                })
                continue
                
            code = file_data["code"]
            language = file_data.get("language")
            filename = file_data.get("filename", f"file_{idx}")
            
            # Auto-detect language if needed
            if not language:
                with tempfile.NamedTemporaryFile(mode='w', suffix=f"_{filename}", delete=False) as temp_file:
                    temp_file.write(code)
                    temp_file_path = temp_file.name
                
                try:
                    language = detect_language(Path(temp_file_path))
                    if not language:
                        errors.append({
                            "index": idx,
                            "filename": filename,
                            "error": "Could not detect language"
                        })
                        continue
                finally:
                    try:
                        os.unlink(temp_file_path)
                    except:
                        pass
            
            try:
                request_obj = AnalysisRequest(
                    code=code,
                    language=language,
                    file_name=filename,
                    options=options
                )
                
                result = orch.analyze(request_obj)
                results.append(format_json_output(result))
            except Exception as e:
                errors.append({
                    "index": idx,
                    "filename": filename,
                    "error": str(e)
                })
        
        response = {
            "results": results,
            "total_analyzed": len(results),
            "total_files": len(data["files"])
        }
        
        if errors:
            response["errors"] = errors
        
        return jsonify(response), 200
    
    except Exception as e:
        logger.exception("Batch analysis failed")
        return jsonify({"error": "Internal server error", "message": str(e)}), 500


@app.route("/info", methods=["GET"])
def get_info():
    """Get analyzer information and capabilities."""
    info = {
        "name": "Big O Complexity Analyzer",
        "version": "1.0.0",
        "supported_languages": [
            "python",
            "java",
            "c",
            "javascript"
        ],
        "complexity_classes": {
            "time": [
                "O(1)",
                "O(log n)",
                "O(n)",
                "O(n log n)",
                "O(n²)",
                "O(n³)",
                "O(2^n)",
                "O(n!)"
            ],
            "space": [
                "O(1)",
                "O(log n)",
                "O(n)",
                "O(n²)",
                "O(2^n)"
            ]
        },
        "features": [
            "Native language analyzers",
            "Pattern recognition",
            "Best/Average/Worst case analysis",
            "Confidence scoring"
        ],
        "limits": {
            "max_batch_size": 50,
            "max_file_size": "1MB"
        }
    }
    return jsonify(info), 200


@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors."""
    return jsonify({
        "error": "Endpoint not found",
        "available_endpoints": ["/", "/health", "/analyze", "/batch", "/info"]
    }), 404


@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors."""
    logger.error(f"Internal server error: {error}")
    return jsonify({"error": "Internal server error"}), 500


@app.before_request
def log_request():
    """Log incoming requests in production."""
    if os.getenv('API_MODE') == 'true':
        logger.info(f"{request.method} {request.path} from {request.remote_addr}")


def create_app():
    """Application factory pattern."""
    return app


if __name__ == "__main__":
    # This should not be used in production
    logger.warning("Running Flask development server - NOT for production!")
    app.run(
        host=os.getenv("API_HOST", "0.0.0.0"),
        port=int(os.getenv("API_PORT", "5000")),
        debug=False
    )