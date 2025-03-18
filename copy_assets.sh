#!/bin/bash

# Check if the assets directory exists in the build directory
if [ ! -d "build/assets" ]; then
    echo "Copying assets to build directory..."
    mkdir -p build/assets
    cp -r assets/* build/assets/
else
    echo "Assets directory already exists in build directory."
fi 