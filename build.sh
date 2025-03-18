#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Run CMake
cmake ..

# Build the project
make

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Running PixelPets..."
    # Run the application
    ./pixelpets
else
    echo "Build failed. Please check the error messages above."
fi 