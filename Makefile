.PHONY: help build run test clean install lint format

help:
	@echo "Available commands:"
	@echo "  make build    - Build Docker image"
	@echo "  make run      - Run analyzer"
	@echo "  make test     - Run tests"
	@echo "  make clean    - Clean temporary files"
	@echo "  make install  - Install dependencies"
	@echo "  make lint     - Run linting"
	@echo "  make format   - Format code"

build:
	docker build -t bigo-analyzer:latest .

run:
	@echo "Usage: make run FILE=<path_to_file>"
	docker run --rm -v $(PWD):/app/input bigo-analyzer:latest analyze /app/input/$(FILE)

test:
	pytest tests/ -v --cov=src

clean:
	find . -type d -name "__pycache__" -exec rm -rf {} +
	find . -type f -name "*.pyc" -delete
	rm -rf .pytest_cache .coverage htmlcov

install:
	pip install -r requirements.txt

lint:
	flake8 src/
	mypy src/

format:
	black src/
	isort src/