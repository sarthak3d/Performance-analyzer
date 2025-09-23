FROM ubuntu:22.04 AS base

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Set working directory
WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Python
    python3.11 \
    python3-pip \
    python3.11-dev \
    # Java
    openjdk-17-jdk \
    maven \
    # C/C++ and Clang
    gcc \
    g++ \
    cmake \
    make \
    clang-14 \
    libclang-14-dev \
    llvm-14 \
    # JSON library for C++
    nlohmann-json3-dev \
    # Node.js for JavaScript
    curl \
    # General utilities
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js 18
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
    apt-get install -y nodejs

# Install Python dependencies
COPY requirements.txt .
RUN pip3 install --no-cache-dir -r requirements.txt

# Build Java analyzer
COPY analyzers/java /app/analyzers/java
WORKDIR /app/analyzers/java
RUN mvn clean package

# Build C++ analyzer
COPY analyzers/cpp /app/analyzers/cpp
WORKDIR /app/analyzers/cpp
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make

# Install JavaScript analyzer dependencies
COPY analyzers/javascript /app/analyzers/javascript
WORKDIR /app/analyzers/javascript
RUN npm install

# Copy Python analyzer
COPY analyzers/python /app/analyzers/python

# Copy main application
WORKDIR /app
COPY src/ ./src/
COPY knowledge_base/ ./knowledge_base/
COPY config/ ./config/
COPY setup.py .

# Install the main application
RUN pip3 install -e .

# Set environment variables
ENV PYTHONPATH=/app
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
ENV PATH=$JAVA_HOME/bin:$PATH

# Create volume mount points
VOLUME ["/app/input", "/app/output"]

# Default command
ENTRYPOINT ["python3", "-m", "src.main"]
CMD ["--help"]