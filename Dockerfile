# ===== BUILD STAGE =====
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

# Install all build dependencies in a single layer
RUN apt-get update && apt-get install -y \
    python3.11 python3-pip python3.11-dev \
    openjdk-17-jdk maven \
    cmake make \
    clang-14 libclang-14-dev llvm-14 llvm-14-dev \
    libllvm14 llvm-14-tools clang-tools-14 libclang-cpp14-dev \
    pkg-config libcjson-dev \
    curl git wget \
    && curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/* 

# Set LLVM/Clang paths for building
ENV LLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm
ENV Clang_DIR=/usr/lib/llvm-14/lib/cmake/clang

# Install Python dependencies
COPY requirements.txt .
RUN pip3 install --no-cache-dir -r requirements.txt && \
    rm requirements.txt

# Build Java analyzer
COPY analyzers/java /app/analyzers/java
RUN cd /app/analyzers/java && mvn clean package

# Build C analyzer
COPY analyzers/c /app/analyzers/c
RUN cd /app/analyzers/c && \
    mkdir -p build && cd build && \
    cmake .. && \
    make 

# Install JavaScript analyzer dependencies
COPY analyzers/javascript /app/analyzers/javascript
# Use npm install if package-lock.json doesn't exist, otherwise use npm ci
RUN cd /app/analyzers/javascript && \
    npm ci --omit=dev

# Copy Python analyzer (no build needed)
COPY analyzers/python /app/analyzers/python

# Copy and install main application
COPY src/ ./src/
COPY knowledge_base/ ./knowledge_base/
COPY config/ ./config/
COPY setup.py .
RUN pip3 install -e .

# ===== RUNTIME STAGE =====
FROM ubuntu:22.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

# Install only runtime dependencies in a single layer
RUN apt-get update && apt-get install -y \
    python3.11 python3-pip \
    openjdk-17-jre-headless \
    libcjson1 \
    libclang1-14 \
    curl \
    && curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs \
    && rm -f /tmp/nodesource_setup.sh \
    && rm -rf /etc/apt/sources.list.d/nodesource.list* \
    && apt purge -y --auto-remove curl \
    && apt autoremove -y \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Install Python dependencies
COPY requirements.txt .
RUN pip3 install --no-cache-dir -r requirements.txt && \
    rm requirements.txt

# Copy Python packages and binaries from builder
COPY --from=builder /usr/local/lib/python3.11/dist-packages /usr/local/lib/python3.11/dist-packages
COPY --from=builder /usr/local/bin /usr/local/bin

# Copy built Java analyzer (only the JAR file, not source)
COPY --from=builder /app/analyzers/java/target/*.jar /app/analyzers/java/target/

# Copy built C analyzer (the correct binary name)
COPY --from=builder /app/analyzers/c/build/c_analyzer /app/analyzers/c/build/

# Copy JavaScript analyzer with dependencies
COPY --from=builder /app/analyzers/javascript /app/analyzers/javascript

# Copy Python analyzer
COPY --from=builder /app/analyzers/python /app/analyzers/python

# Copy application code and configuration
COPY --from=builder /app/src /app/src
COPY --from=builder /app/knowledge_base /app/knowledge_base
COPY --from=builder /app/config /app/config
COPY --from=builder /app/setup.py /app/setup.py

# Install the application (should be quick as dependencies are already copied)
RUN pip3 install -e .

# Set environment variables
ENV PYTHONPATH=/app
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
ENV PATH=$JAVA_HOME/bin:$PATH

# Create volume mount points
VOLUME ["/input", "/output"]

# Default to CLI mode
ENV API_MODE=false
ENV API_PORT=5000
ENV API_HOST=0.0.0.0
ENV API_WORKERS=1
ENV API_TIMEOUT=120
ENV API_MAX_REQUESTS=1000
ENV API_MAX_REQUESTS_JITTER=50

EXPOSE 5000

# Use Python as entrypoint for both CLI and API modes
ENTRYPOINT ["python3", "-m", "src.main"]
CMD []

