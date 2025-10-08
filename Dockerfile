# Dockerfile for AF_HTTP
# Build and run the AF_HTTP server
# Designed for Docker Swarm deployment

FROM alpine:latest

# Install required build tools
RUN apk add --no-cache gcc musl-dev make git

WORKDIR /app

# Clone the GitHub repository
RUN git clone https://github.com/jhallDevelop/AF_Docs.git .

# Initialize and update submodules
RUN git submodule update --init --recursive

# Build the main project
RUN make

# Build the AF_HTTP library
RUN cd libs/AF_HTTP && make

# Expose port 8080
EXPOSE 8080

# Run the AF_HTTP server from the public directory
WORKDIR /app/public
CMD ["../libs/AF_HTTP/bin/AF_HTTP", "8080"]