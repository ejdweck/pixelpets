# PixelPets

A nostalgic pixel art style game inspired by early 2000s GameBoy games, designed to run on an ESP32 with a LILYGO T3 AMOLED screen.

## Project Overview

This project is a proof of concept for a hardware device that will eventually run on an ESP32 board with a LILYGO T3 AMOLED screen. The game features pixel art style graphics to invoke nostalgia of early 2000s GameBoy games.

## Features

- Pixel art graphics
- Touch screen interface (simulated with mouse in the proof of concept)
- WiFi connectivity (planned for future updates from a web server)

## Development Environment

### Proof of Concept (PC/Mac)
- SDL2 for graphics rendering and input handling
- CMake for build system

### Hardware Implementation (Future)
- ESP32 board
- LILYGO T3 AMOLED screen
- Arduino IDE or PlatformIO
- TFT_eSPI or LovyanGFX for display
- WiFiManager for WiFi connectivity
- ArduinoJSON for parsing updates

## Building the Proof of Concept

### Prerequisites
- CMake (3.10 or higher)
- SDL2 library

### On macOS
```bash
# Install dependencies (if not already installed)
brew install cmake sdl2

# Build the project
mkdir build
cd build
cmake ..
make

# Run the application
./pixelpets
```

### On Linux
```bash
# Install dependencies
sudo apt-get install cmake libsdl2-dev

# Build the project
mkdir build
cd build
cmake ..
make

# Run the application
./pixelpets
```

### On Windows
```bash
# Install dependencies using vcpkg or similar
# Build using CMake GUI or command line
```

## Controls

- Click/touch to interact
- Press ESC to exit

## Future Plans

1. Port to ESP32 with LILYGO T3 AMOLED screen
2. Implement WiFi connectivity for updates
3. Add game mechanics and more pixel art assets
4. Create a full game experience 