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
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/* || true

# Correctly build and install Google Test
WORKDIR /usr/src/gtest
RUN cmake . && make && cp lib/*.a /usr/lib

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

# Copy the source code and header files
COPY adserver.cpp .
COPY adserver.hpp .
COPY adserver_test.cpp .

RUN mkdir -p /mnt/adserver_geodata
# Copy the geodata folder into the container
COPY geodata /mnt/adserver_geodata/

# Verify that files exist inside the container
RUN ls -lah /mnt/adserver_geodata/

# Compile the C++ application (Full Version with Main)
RUN g++ -std=c++17 -o adserver adserver.cpp -I/usr/local/include -L/usr/local/lib \
    -lboost_system -lboost_json -lpthread

# Compile the C++ application (Without Main for Testing)
RUN g++ -std=c++17 -c adserver.cpp -I/usr/local/include -o adserver.o -DUNIT_TESTING

# Compile the tests, linking with adserver.o but NOT adserver's main()
RUN g++ -std=c++17 -o adserver_test adserver_test.cpp adserver.o -I/usr/local/include -L/usr/local/lib \
    -lgtest -lgtest_main -lpthread -lboost_system -lboost_json

# Expose the required port
EXPOSE 80

# Run the tests first, then start the server if tests pass
CMD ["/bin/sh", "-c", "export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH && /app/adserver_test && exec /app/adserver"]