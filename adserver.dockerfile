FROM ubuntu:20.04

# Set noninteractive mode to avoid prompts
ENV DEBIAN_FRONTEND=noninteractive

# Ensure Boost libraries are found
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH  

# Install system dependencies including ca-certificates for SSL verification
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    curl \
    dnsutils \
    cmake \
    wget \
    tar \
    && rm -rf /var/lib/apt/lists/* || true

# Download and extract Boost 1.82
WORKDIR /tmp
RUN wget -q https://archives.boost.io/release/1.82.0/source/boost_1_82_0.tar.gz -O boost.tar.gz && \
    tar -xf boost.tar.gz

# Build and install Boost with explicit toolset (installing only needed libraries)
WORKDIR /tmp/boost_1_82_0
RUN ./bootstrap.sh --with-toolset=gcc
RUN ./b2 install --prefix=/usr/local --with-json --with-system --with-thread --layout=system

# Set working directory for the application
WORKDIR /app

# Copy the source code and header files (removed adserver_test.cpp)
COPY adserver.cpp .
COPY adserver.hpp .

# Create directory and copy geodata into the container
RUN mkdir -p /mnt/adserver_geodata
COPY geodata /mnt/adserver_geodata/

# Verify that geodata exists inside the container
RUN ls -lah /mnt/adserver_geodata/

# Compile the C++ application (production build with main())
RUN g++ -std=c++17 -o adserver adserver.cpp -I/usr/local/include -L/usr/local/lib \
    -lboost_system -lboost_json -lpthread

# Expose the required port
EXPOSE 80

# Start the adserver binary
CMD ["/bin/sh", "-c", "export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH && exec /app/adserver"]