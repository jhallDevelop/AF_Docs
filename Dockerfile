# Dockerfile for AF_HTTP
# Build and run the AF_HTTP server
# Designed for Docker Swarm deployment

FROM alpine:latest

# Install required build tools
RUN apk add --no-cache gcc musl-dev make git

WORKDIR /app

# Clone the GitHub repository
RUN git clone https://github.com/jhallDevelop/AF_Docs.git .

# Debug: Show what was cloned
RUN echo "Root directory contents:" && ls -la

# Debug: Check if .gitmodules exists
RUN echo ".gitmodules contents:" && cat .gitmodules || echo ".gitmodules not found"

# Initialize and update submodules with verbose output
RUN echo "=== Running git submodule update ===" && \
    git submodule update --init --recursive --verbose 2>&1 || echo "Submodule update failed"

# Debug: Show what's in libs after submodule update
RUN ls -la libs/ && \
    echo "=== MD4C ===" && ls -la libs/md4c/ && \
    echo "=== MD4C/SRC ===" && ls -la libs/md4c/src/ && \
    echo "=== AF_HTTP ===" && ls -la libs/AF_HTTP/

# Build the main project
RUN make

# Build the AF_HTTP library
RUN cd libs/AF_HTTP && make

# Expose port 8080
EXPOSE 8080

# Run the AF_HTTP server from the public directory
WORKDIR /app/public
CMD ["../libs/AF_HTTP/bin/AF_HTTP", "8080"]