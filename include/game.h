#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <random>

// Constants for the LILYGO T3 AMOLED screen
// Updated to match the physical dimensions shown in the screenshot
const int SCREEN_WIDTH = 135;  // Width in pixels
const int SCREEN_HEIGHT = 240; // Height in pixels
const int PIXEL_SIZE = 2;      // Reduced pixel size for better detail on smaller screen

// Menu constants
const int MENU_BUTTON_SIZE = 24;
const int BG_BUTTON_SIZE = 24;
const int MENU_GRID_COLS = 3;  // Updated to show more plants
const int MENU_GRID_ROWS = 4;  // Updated to show more plants
const int MENU_ITEM_PADDING = 5; // Reduced padding to fit more plants

// Plant gift timer (10 seconds)
const int PLANT_GIFT_INTERVAL = 10000; // in milliseconds

// Celebration animation constants
const int CELEBRATION_DURATION = 2000; // Duration of celebration animation in milliseconds
const int MAX_PARTICLES = 50;          // Maximum number of celebration particles
const int PARTICLE_SIZE = 3;           // Size of celebration particles

// Game states
enum class GameState {
    INTRO,                // Intro screen 
    NAME_ENTRY,           // Enter player name
    GENDER_SELECTION,     // Select player gender
    STARTER_SELECTION,    // Choose starter plant
    PLANT_VIEW,           // Viewing a single plant
    MENU_VIEW,            // Viewing the menu grid of plants
    CELEBRATION_ANIMATION,// Celebration animation before gift notification
    GIFT_NOTIFICATION     // Notification of new plant gift
};

// Background types
enum class BackgroundType {
    SUNNY,
    RAINY,
    CLOUDY,
    NIGHT
};

// Background type names array
const std::string BackgroundTypeNames[] = {
    "Sunny",
    "Rainy",
    "Cloudy",
    "Night"
};

// Plant data structure
struct Plant {
    std::string name;
    std::string filename;
    SDL_Texture* texture;
    int width;
    int height;
    BackgroundType preferredBackground = BackgroundType::SUNNY; // Default background
    bool isOwned = false; // Whether the player owns this plant
};

// Button structure
struct Button {
    SDL_Rect rect;
    bool isPressed;
    
    bool contains(int x, int y) const {
        return (x >= rect.x && x < rect.x + rect.w &&
                y >= rect.y && y < rect.y + rect.h);
    }
};

// Color palette - GameBoy inspired
struct ColorPalette {
    SDL_Color background = {15, 56, 15, 255};    // Dark green
    SDL_Color darkest = {48, 98, 48, 255};       // Medium green
    SDL_Color medium = {139, 172, 15, 255};      // Light green
    SDL_Color lightest = {155, 188, 15, 255};    // Pale green
    
    // Additional colors
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};
};

// Background data structure
struct Background {
    std::string name;
    std::string filename;
    SDL_Texture* texture;
    int width;
    int height;
};

// Gender type
enum class Gender {
    MALE,
    FEMALE
};

// Player data structure
struct Player {
    std::string name = "";
    Gender gender = Gender::MALE;
    int selectedPlantIndex = -1;  // Index of chosen starter plant
    std::vector<int> ownedPlants; // Indices of owned plants
    Uint32 lastGiftTime = 0;      // Last time player received a plant gift
};

// Celebration particle structure
struct Particle {
    float x;
    float y;
    float velocityX;
    float velocityY;
    SDL_Color color;
    int size;
    int lifespan;
    int age;
};

// Function to draw a pixel at (x, y) with the given color
inline void drawPixel(SDL_Renderer* renderer, int x, int y, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect = {x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE};
    SDL_RenderFillRect(renderer, &rect);
}

// Function to load a texture from a file
inline SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    // Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    // Create texture from surface pixels
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (newTexture == nullptr) {
        std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
    }
    
    // Get rid of old loaded surface
    SDL_FreeSurface(loadedSurface);
    
    return newTexture;
}

// Function to render a texture to the screen
inline void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, SDL_Rect* clip = nullptr, double scale = 1.0) {
    // Enable alpha blending for transparency
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    
    // Set rendering space
    SDL_Rect renderQuad = {x, y, 0, 0};
    
    // Set clip rendering dimensions
    if (clip != nullptr) {
        renderQuad.w = clip->w * scale;
        renderQuad.h = clip->h * scale;
    } else {
        // Query texture to get its width and height
        SDL_QueryTexture(texture, nullptr, nullptr, &renderQuad.w, &renderQuad.h);
        renderQuad.w *= scale;
        renderQuad.h *= scale;
    }
    
    // Render to screen
    SDL_RenderCopy(renderer, texture, clip, &renderQuad);
}

// Function to draw a button
inline void drawButton(SDL_Renderer* renderer, const Button& button, const ColorPalette& palette) {
    // Draw button background
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &button.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &button.rect);
    
    // Draw menu icon (three horizontal lines)
    int lineWidth = button.rect.w * 0.6;
    int lineHeight = 2;
    int lineSpacing = 4;
    int startX = button.rect.x + (button.rect.w - lineWidth) / 2;
    int startY = button.rect.y + (button.rect.h - (3 * lineHeight + 2 * lineSpacing)) / 2;
    
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    
    for (int i = 0; i < 3; i++) {
        SDL_Rect line = {startX, startY + i * (lineHeight + lineSpacing), lineWidth, lineHeight};
        SDL_RenderFillRect(renderer, &line);
    }
}

// Function to draw the background cycle button
inline void drawBgButton(SDL_Renderer* renderer, const Button& button, const ColorPalette& palette) {
    // Draw button background
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &button.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &button.rect);
    
    // Draw background cycle icon (a simple sun and cloud)
    int centerX = button.rect.x + button.rect.w / 2;
    int centerY = button.rect.y + button.rect.h / 2;
    int radius = button.rect.w / 4;
    
    // Draw sun
    SDL_SetRenderDrawColor(renderer, palette.yellow.r, palette.yellow.g, palette.yellow.b, palette.yellow.a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                SDL_RenderDrawPoint(renderer, centerX - 5 + x, centerY + y);
            }
        }
    }
    
    // Draw cloud
    SDL_SetRenderDrawColor(renderer, palette.white.r, palette.white.g, palette.white.b, palette.white.a);
    for (int i = 0; i < 2; i++) {
        for (int y = -3; y <= 3; y++) {
            for (int x = -3; x <= 3; x++) {
                if (x*x + y*y <= 9) {
                    SDL_RenderDrawPoint(renderer, centerX + 3 + x + i*4, centerY - 2 + y);
                }
            }
        }
    }
}

// Function to draw text input field
inline void drawTextInputField(SDL_Renderer* renderer, const std::string& text, int x, int y, int width, int height, const ColorPalette& palette, bool isActive) {
    // Draw input field background
    SDL_Rect inputRect = {x, y, width, height};
    
    if (isActive) {
        SDL_SetRenderDrawColor(renderer, palette.white.r, palette.white.g, palette.white.b, palette.white.a);
    } else {
        SDL_SetRenderDrawColor(renderer, palette.lightest.r, palette.lightest.g, palette.lightest.b, palette.lightest.a);
    }
    
    SDL_RenderFillRect(renderer, &inputRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &inputRect);
    
    // Draw text
    // Use the existing drawPixelText function, which is defined in main.cpp
    // This function will be called from main.cpp
}

// Function to draw a gender selection button
inline void drawGenderButton(SDL_Renderer* renderer, const Button& button, bool isMale, bool isSelected, const ColorPalette& palette) {
    // Draw button background
    if (isSelected) {
        SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    } else {
        SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    }
    SDL_RenderFillRect(renderer, &button.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    SDL_RenderDrawRect(renderer, &button.rect);
    
    // Draw gender symbol (simplistic)
    int centerX = button.rect.x + button.rect.w / 2;
    int centerY = button.rect.y + button.rect.h / 2;
    int radius = button.rect.w / 4;
    
    if (isMale) {
        // Male symbol (circle with arrow)
        SDL_SetRenderDrawColor(renderer, palette.blue.r, palette.blue.g, palette.blue.b, palette.blue.a);
        
        // Draw circle
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius && x*x + y*y >= (radius-2)*(radius-2)) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
        
        // Draw arrow
        SDL_RenderDrawLine(renderer, centerX, centerY - radius, centerX + radius, centerY - radius - radius);
        SDL_RenderDrawLine(renderer, centerX + radius, centerY - radius - radius, centerX + radius - 4, centerY - radius - radius + 4);
        SDL_RenderDrawLine(renderer, centerX + radius, centerY - radius - radius, centerX + radius - 4, centerY - radius - radius - 4);
    } else {
        // Female symbol (circle with cross)
        SDL_SetRenderDrawColor(renderer, palette.red.r, palette.red.g, palette.red.b, palette.red.a);
        
        // Draw circle
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= radius*radius && x*x + y*y >= (radius-2)*(radius-2)) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
        
        // Draw cross below
        SDL_RenderDrawLine(renderer, centerX, centerY + radius, centerX, centerY + radius + radius/2);
        SDL_RenderDrawLine(renderer, centerX - radius/2, centerY + radius + radius/4, centerX + radius/2, centerY + radius + radius/4);
    }
}

// Function to draw a plant selection button with a highlight if selected
inline void drawPlantSelectionButton(SDL_Renderer* renderer, const Button& button, SDL_Texture* plantTexture, int plantWidth, int plantHeight, bool isSelected, const ColorPalette& palette) {
    // Draw button background
    if (isSelected) {
        SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    } else {
        SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    }
    SDL_RenderFillRect(renderer, &button.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    SDL_RenderDrawRect(renderer, &button.rect);
    
    // Draw plant texture (scaled to fit button)
    if (plantTexture) {
        double scaleWidth = (button.rect.w - 10) / static_cast<double>(plantWidth);
        double scaleHeight = (button.rect.h - 10) / static_cast<double>(plantHeight);
        double scale = std::min(scaleWidth, scaleHeight);
        
        int scaledWidth = plantWidth * scale;
        int scaledHeight = plantHeight * scale;
        int x = button.rect.x + (button.rect.w - scaledWidth) / 2;
        int y = button.rect.y + (button.rect.h - scaledHeight) / 2;
        
        renderTexture(renderer, plantTexture, x, y, nullptr, scale);
    }
    
    // Draw highlight if selected
    if (isSelected) {
        // Draw a small highlight border inside the button
        SDL_Rect highlightRect = {
            button.rect.x + 2, 
            button.rect.y + 2, 
            button.rect.w - 4, 
            button.rect.h - 4
        };
        SDL_SetRenderDrawColor(renderer, palette.yellow.r, palette.yellow.g, palette.yellow.b, palette.yellow.a);
        SDL_RenderDrawRect(renderer, &highlightRect);
    }
}

// Function to draw navigation buttons for the plant menu
inline void drawNavButtons(SDL_Renderer* renderer, const Button& prevButton, const Button& nextButton, const ColorPalette& palette) {
    // Draw prev button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &prevButton.rect);
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &prevButton.rect);
    
    // Draw prev arrow
    int arrowSize = 8;
    int centerX = prevButton.rect.x + prevButton.rect.w / 2;
    int centerY = prevButton.rect.y + prevButton.rect.h / 2;
    
    SDL_Point prevArrow[3] = {
        {centerX + arrowSize/2, centerY - arrowSize/2},
        {centerX - arrowSize/2, centerY},
        {centerX + arrowSize/2, centerY + arrowSize/2}
    };
    
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawLine(renderer, prevArrow[i].x, prevArrow[i].y, prevArrow[i+1].x, prevArrow[i+1].y);
    }
    SDL_RenderDrawLine(renderer, prevArrow[0].x, prevArrow[0].y, prevArrow[2].x, prevArrow[2].y);
    
    // Draw next button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &nextButton.rect);
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &nextButton.rect);
    
    // Draw next arrow
    centerX = nextButton.rect.x + nextButton.rect.w / 2;
    centerY = nextButton.rect.y + nextButton.rect.h / 2;
    
    SDL_Point nextArrow[3] = {
        {centerX - arrowSize/2, centerY - arrowSize/2},
        {centerX + arrowSize/2, centerY},
        {centerX - arrowSize/2, centerY + arrowSize/2}
    };
    
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    for (int i = 0; i < 2; i++) {
        SDL_RenderDrawLine(renderer, nextArrow[i].x, nextArrow[i].y, nextArrow[i+1].x, nextArrow[i+1].y);
    }
    SDL_RenderDrawLine(renderer, nextArrow[0].x, nextArrow[0].y, nextArrow[2].x, nextArrow[2].y);
}

// Function to draw a notification box
inline void drawNotificationBox(SDL_Renderer* renderer, const std::string& title, const std::string& message, 
                               const SDL_Rect& rect, const ColorPalette& palette) {
    // Draw box background
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &rect);
    
    // Draw box border
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &rect);
    
    // Title and message will be drawn in main.cpp using drawPixelText
}

// Function to get three random indices from a range
inline std::vector<int> getRandomIndices(int min, int max, int count) {
    std::vector<int> indices;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    
    // Create a set to ensure uniqueness
    std::set<int> uniqueIndices;
    while (uniqueIndices.size() < count) {
        uniqueIndices.insert(distrib(gen));
    }
    
    // Convert set to vector
    indices.assign(uniqueIndices.begin(), uniqueIndices.end());
    return indices;
}

// Function to draw a celebration particle
inline void drawParticle(SDL_Renderer* renderer, const Particle& particle) {
    SDL_SetRenderDrawColor(renderer, particle.color.r, particle.color.g, particle.color.b, 
                           particle.color.a * (1.0f - static_cast<float>(particle.age) / particle.lifespan));
    
    SDL_Rect particleRect = {
        static_cast<int>(particle.x), 
        static_cast<int>(particle.y), 
        particle.size, 
        particle.size
    };
    
    SDL_RenderFillRect(renderer, &particleRect);
}

// Function to initialize a random celebration particle
inline Particle createRandomParticle(int screenWidth, int screenHeight) {
    Particle particle;
    // Start from the center of the screen
    particle.x = screenWidth / 2.0f;
    particle.y = screenHeight / 2.0f;
    
    // Random velocity (radial burst pattern)
    float angle = (std::rand() % 360) * 3.14159f / 180.0f;
    float speed = 0.5f + (std::rand() % 20) / 10.0f;
    particle.velocityX = cos(angle) * speed;
    particle.velocityY = sin(angle) * speed;
    
    // Random color (festive colors)
    int colorChoice = std::rand() % 5;
    switch (colorChoice) {
        case 0: particle.color = {255, 0, 0, 255};   break; // Red
        case 1: particle.color = {255, 255, 0, 255}; break; // Yellow
        case 2: particle.color = {0, 255, 0, 255};   break; // Green
        case 3: particle.color = {0, 0, 255, 255};   break; // Blue
        case 4: particle.color = {255, 0, 255, 255}; break; // Purple
    }
    
    // Random size and lifespan
    particle.size = 2 + std::rand() % 3;
    particle.lifespan = 30 + std::rand() % 60; // frames
    particle.age = 0;
    
    return particle;
}

#endif // GAME_H 