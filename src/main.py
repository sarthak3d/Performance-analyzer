"""
Main entry point for Big O Analyzer
Supports both CLI and API modes
"""
import os
import sys
from loguru import logger

def main():
    """Main entry point."""
    api_mode = os.getenv("API_MODE", "false").lower() == "true"
    
    if api_mode:
        start_api_server()
    else:
        start_cli()


def start_api_server():
    """Start the API server in appropriate mode."""
    from .api import app
    
    api_host = os.getenv("API_HOST", "0.0.0.0")
    api_port = int(os.getenv("API_PORT", "5000"))
    workers = int(os.getenv("API_WORKERS", "1"))
    
    # Check if we should use production server
    use_production = os.getenv("USE_GUNICORN", "true").lower() == "true"
    
    if use_production:
        # Use Gunicorn for production
        logger.info("Starting Big O Analyzer API with Gunicorn (production mode)...")
        
        try:
            from gunicorn.app.base import BaseApplication
            
            class StandaloneApplication(BaseApplication):
                def __init__(self, app, options=None):
                    self.options = options or {}
                    self.application = app
                    super().__init__()
                
                def load_config(self):
                    config = {
                        key: value for key, value in self.options.items()
                        if key in self.cfg.settings and value is not None
                    }
                    for key, value in config.items():
                        self.cfg.set(key.lower(), value)
                
                def load(self):
                    return self.application
            
            # Gunicorn options
            options = {
                'bind': f'{api_host}:{api_port}',
                'workers': workers,
                'worker_class': 'sync',
                'timeout': int(os.getenv("API_TIMEOUT", "120")),
                'max_requests': int(os.getenv("API_MAX_REQUESTS", "1000")),
                'max_requests_jitter': int(os.getenv("API_MAX_REQUESTS_JITTER", "50")),
                'accesslog': '-',
                'errorlog': '-',
                'loglevel': os.getenv("LOG_LEVEL", "info"),
                'preload_app': False,
                'proc_name': 'performance-analyzer-api',
            }
            
            StandaloneApplication(app, options).run()
            
        except ImportError:
            logger.error("Gunicorn not installed. Install with: pip install gunicorn")
            logger.info("Falling back to Flask development server...")
            start_flask_dev_server(app, api_host, api_port)
    else:
        # Development mode with Flask
        logger.warning("Starting Big O Analyzer API with Flask development server...")
        logger.warning("This is NOT recommended for production!")
        start_flask_dev_server(app, api_host, api_port)


def start_flask_dev_server(app, host, port):
    """Start Flask development server."""
    app.run(
        host=host,
        port=port,
        debug=False,
        threaded=True
    )


def start_cli():
    """Start the CLI application."""
    from .cli import create_app
    
    cli_app = create_app()
    cli_app()


if __name__ == "__main__":
    main()