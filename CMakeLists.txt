cmake_minimum_required(VERSION 3.10)
project(PixelPets VERSION 0.1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set module path to find our custom FindSDL2_image.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Find SDL2_image package
find_package(SDL2_image REQUIRED)

# Find SDL2_ttf package
find_package(SDL2_ttf REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} include)

# Add explicit include directories for macOS
include_directories(/opt/homebrew/include)

# Add explicit library directories for macOS
link_directories(/opt/homebrew/lib)

# Add executable
add_executable(pixelpets src/main.cpp src/render.cpp)

# Link libraries
target_link_libraries(pixelpets 
    ${SDL2_LIBRARIES} 
    ${SDL2_IMAGE_LIBRARIES} 
    "-framework CoreVideo" 
    "-framework CoreFoundation"
    SDL2_ttf
)

# If SDL2_IMAGE_LIBRARIES is not set, try to find it manually
if(NOT SDL2_IMAGE_LIBRARIES)
    find_library(SDL2_IMAGE_LIBRARY
        NAMES SDL2_image
        HINTS
        ENV SDL2IMAGEDIR
        ENV SDL2DIR
        PATH_SUFFIXES lib
    )
    if(SDL2_IMAGE_LIBRARY)
        set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
    endif()
endif()

# If SDL2_TTF_LIBRARIES is not set, try to find it manually
if(NOT SDL2_TTF_LIBRARIES)
    find_library(SDL2_TTF_LIBRARY
        NAMES SDL2_ttf
        HINTS
        ENV SDL2TTFDIR
        ENV SDL2DIR
        PATH_SUFFIXES lib
    )
    if(SDL2_TTF_LIBRARY)
        set(SDL2_TTF_LIBRARIES ${SDL2_TTF_LIBRARY})
    endif()
endif()

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}) 