# Use an official C++ base image
FROM gcc:latest

# Install CMake and Git
RUN apt-get update && \
    apt-get install -y cmake git gcc-arm-none-eabi && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Clone your GitHub repository
RUN git clone https://github.com/kylaro/firefly-led-framework .

# Specify the command to run your build
CMD ["bash", "-c", "cmake -S . -B build && exec bash"]