from setuptools import setup, find_packages

# Simple setup without README dependency
setup(
    name="performance-analyzer",
    version="1.0.0",
    author="Sarthak Darshan",
    description="Professional Big O Complexity Analyzer",
    long_description="A professional-grade tool for analyzing time and space complexity of code across multiple programming languages using native language analyzers.",
    long_description_content_type="text/plain",
    packages=find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.8",
    install_requires=[
        "click>=8.0",
        "typer>=0.9.0",
        "rich>=13.0",
        "pydantic>=2.0",
        "loguru>=0.7",
    ],
    entry_points={
        "console_scripts": [
            "bigo=src.main:main",
        ],
    },
)