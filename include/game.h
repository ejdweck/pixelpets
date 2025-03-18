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

// Weather and time constants
const int WEATHER_CHANGE_INTERVAL = 30000;  // 30 seconds in milliseconds
const int TOOLBAR_HEIGHT = 40;  // Height of the toolbar at bottom

// Menu constants
const int MENU_BUTTON_SIZE = 24;
const int BG_BUTTON_SIZE = 24;
const int SELL_BUTTON_SIZE = 40;  // Size of sell plant button
const int STORE_BUTTON_SIZE = 24; // Size of store button
const int MENU_GRID_COLS = 3;  // Updated to show more plants
const int MENU_GRID_ROWS = 4;  // Updated to show more plants
const int MENU_ITEM_PADDING = 5; // Reduced padding to fit more plants

// Celebration animation constants
const int CELEBRATION_DURATION = 2000; // Duration of celebration animation in milliseconds
const int MAX_PARTICLES = 50;          // Maximum number of celebration particles
const int PARTICLE_SIZE = 3;           // Size of celebration particles

// Game states
enum class GameState {
    INTRO,                // Intro screen 
    STARTER_SELECTION,    // Choose starter plant
    PLANT_VIEW,           // Viewing a single plant
    INVENTORY_VIEW,       // Viewing the inventory grid of plants
    MAP_VIEW,            // Viewing the map of locations
    HOUSE_VIEW,          // Viewing the house
    STORE_VIEW,          // Viewing the store
    PASTURE_VIEW,        // Viewing the pasture
    GREENHOUSE_VIEW,     // Viewing the greenhouse
    CELEBRATION_ANIMATION,// Celebration animation before gift notification
    GIFT_NOTIFICATION,    // Notification of new plant gift
    STORE                 // Plant store to buy new plants
};

// Background types
enum class WeatherType {
    SUNNY,
    RAINY,
    CLOUDY,
    WINDY
};

enum class DayNightType {
    DAY,
    NIGHT
};

// Weather type names array
const std::string WeatherTypeNames[] = {
    "Sunny",
    "Rainy",
    "Cloudy",
    "Windy"
};

// Plant data structure
struct Plant {
    std::string name;
    std::string filename;
    SDL_Texture* texture;
    int width;
    int height;
    WeatherType preferredWeather;
    bool isOwned;
    
    // Default constructor
    Plant() : texture(nullptr), width(0), height(0), preferredWeather(WeatherType::SUNNY), isOwned(false) {}
    
    ~Plant() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
    
    // Prevent copying to avoid double-free
    Plant(const Plant&) = delete;
    Plant& operator=(const Plant&) = delete;
    
    // Allow moving
    Plant(Plant&& other) noexcept
        : name(std::move(other.name))
        , filename(std::move(other.filename))
        , texture(other.texture)
        , width(other.width)
        , height(other.height)
        , preferredWeather(other.preferredWeather)
        , isOwned(other.isOwned) {
        other.texture = nullptr;
    }
    
    Plant& operator=(Plant&& other) noexcept {
        if (this != &other) {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
            name = std::move(other.name);
            filename = std::move(other.filename);
            texture = other.texture;
            width = other.width;
            height = other.height;
            preferredWeather = other.preferredWeather;
            isOwned = other.isOwned;
            other.texture = nullptr;
        }
        return *this;
    }
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
    // Classic GameBoy colors
    // Lightest to darkest (4 shades of green)
    SDL_Color lightest = {155, 188, 15, 255};   // Light green (#9bbc0f)
    SDL_Color medium = {139, 172, 15, 255};     // Medium green (#8bac0f)
    SDL_Color darkest = {48, 98, 48, 255};      // Dark green (#306230) 
    SDL_Color background = {15, 56, 15, 255};   // Darkest green (#0f380f)
    
    // Additional colors
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {228, 228, 208, 255};    // Off-white with greenish tint (#e4e4d0)
    SDL_Color yellow = {255, 255, 100, 255};   // Slightly muted yellow
    SDL_Color red = {220, 50, 50, 255};        // Muted red
    SDL_Color blue = {80, 100, 220, 255};      // Muted blue
    SDL_Color brown = {139, 69, 19, 255};      // Brown for soil
    SDL_Color waterBlue = {30, 144, 255, 255}; // Dodger blue for water
};

// Background data structure
struct Background {
    std::string name;
    std::string filename;
    SDL_Texture* texture;
    int width;
    int height;
    
    // Default constructor
    Background() : texture(nullptr), width(0), height(0) {}
    
    ~Background() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }
    
    // Prevent copying to avoid double-free
    Background(const Background&) = delete;
    Background& operator=(const Background&) = delete;
    
    // Allow moving
    Background(Background&& other) noexcept
        : name(std::move(other.name))
        , filename(std::move(other.filename))
        , texture(other.texture)
        , width(other.width)
        , height(other.height) {
        other.texture = nullptr;
    }
    
    Background& operator=(Background&& other) noexcept {
        if (this != &other) {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
            name = std::move(other.name);
            filename = std::move(other.filename);
            texture = other.texture;
            width = other.width;
            height = other.height;
            other.texture = nullptr;
        }
        return *this;
    }
};

// Structure to represent a raindrop
struct Raindrop {
    float x;
    float y;
    float speed;
    int length;
};

// Player data structure
struct Player {
    int selectedPlantIndex = -1;  // Index of chosen starter plant
    std::vector<int> ownedPlants; // Indices of owned plants
    int coins = 0;  // Player's currency
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

// Add new button definitions for plant navigation
struct PlantNavigationButtons {
    Button prevPlantButton;
    Button nextPlantButton;
    Button mapButton;
    Button storeButton;  // Added store button
};

// Location data structure
struct Location {
    std::string name;
    std::string description;
    SDL_Rect buttonRect;  // Clickable area on the map
    GameState viewState;  // Which game state to switch to when clicked
};

// Store state
struct StoreState {
    bool isAskingToSell = false;
    bool isShowingOffer = false;
    int selectedPlantIndex = -1;
    int offerAmount = 0;
    std::string shopkeeperText = "Welcome to my shop! Would you like to sell any plants?";
    Button yesButton = {{SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT - TOOLBAR_HEIGHT - 40, 40, 20}, false};
    Button noButton = {{SCREEN_WIDTH/2 + 10, SCREEN_HEIGHT - TOOLBAR_HEIGHT - 40, 40, 20}, false};
    Button backButton = {{10, 10, 40, 20}, false};
};

// Game state data structure
struct GameStateData {
    GameState currentState = GameState::INTRO;
    std::vector<Plant> plants;
    Player player;
    StoreState storeState;
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

// Function declarations (non-inline)
void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, SDL_Rect* clip, double scale);
void drawButton(SDL_Renderer* renderer, const Button& button, const ColorPalette& palette);
void drawBgButton(SDL_Renderer* renderer, const Button& button, const ColorPalette& palette);
void drawPlantSelectionButton(SDL_Renderer* renderer, const Button& button, SDL_Texture* plantTexture, int plantWidth, int plantHeight, bool isSelected, const ColorPalette& palette);
void drawFertilizerIcon(SDL_Renderer* renderer, int x, int y, int size, const ColorPalette& palette);

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

// Function to draw a water droplet icon
inline void drawWaterIcon(SDL_Renderer* renderer, int x, int y, int size, const ColorPalette& palette) {
    // Draw water droplet shape
    SDL_SetRenderDrawColor(renderer, palette.waterBlue.r, palette.waterBlue.g, palette.waterBlue.b, palette.waterBlue.a);
    
    // Circle for top of droplet
    int radius = size / 3;
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                SDL_Rect pixel = {x + dx, y + dy, 1, 1};
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
    
    // Triangle for bottom of droplet
    SDL_Point points[3] = {
        {x - radius, y},
        {x + radius, y},
        {x, y + radius * 2}
    };
    
    for (int py = y; py <= y + radius * 2; py++) {
        int width = radius * 2 - (py - y);
        for (int px = x - width/2; px <= x + width/2; px++) {
            SDL_Rect pixel = {px, py, 1, 1};
            SDL_RenderFillRect(renderer, &pixel);
        }
    }
    
    // Draw outline
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawLine(renderer, points[0].x, points[0].y, points[2].x, points[2].y);
    SDL_RenderDrawLine(renderer, points[1].x, points[1].y, points[2].x, points[2].y);
}

// Function to draw a fertilizer icon
inline void drawFertilizerIcon(SDL_Renderer* renderer, int x, int y, int size, const ColorPalette& palette) {
    // Draw fertilizer bag
    SDL_SetRenderDrawColor(renderer, palette.brown.r, palette.brown.g, palette.brown.b, palette.brown.a);
    
    // Draw bag shape
    SDL_Rect bag = {x - size/2, y - size/3, size, size*2/3};
    SDL_RenderFillRect(renderer, &bag);
    
    // Draw N-P-K text on bag
    SDL_SetRenderDrawColor(renderer, palette.white.r, palette.white.g, palette.white.b, palette.white.a);
    
    // Draw simplified N
    SDL_RenderDrawLine(renderer, x - size/4, y - size/6, x - size/4, y + size/6);
    SDL_RenderDrawLine(renderer, x - size/4, y - size/6, x - size/8, y + size/6);
    
    // Draw simplified P
    SDL_RenderDrawLine(renderer, x, y - size/6, x, y + size/6);
    SDL_Rect pCircle = {x, y - size/6, size/8, size/8};
    SDL_RenderDrawRect(renderer, &pCircle);
    
    // Draw simplified K
    SDL_RenderDrawLine(renderer, x + size/4, y - size/6, x + size/4, y + size/6);
    SDL_RenderDrawLine(renderer, x + size/4, y, x + size/3, y - size/6);
    SDL_RenderDrawLine(renderer, x + size/4, y, x + size/3, y + size/6);
    
    // Draw outline
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &bag);
}

// Function to draw a progress bar
inline void drawProgressBar(SDL_Renderer* renderer, int x, int y, int width, int height, 
                            float percentage, const SDL_Color& fillColor, const SDL_Color& emptyColor,
                            const SDL_Color& borderColor) {
    // Clamp percentage between 0 and 100
    percentage = std::max(0.0f, std::min(100.0f, percentage));
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
    SDL_Rect borderRect = {x, y, width, height};
    SDL_RenderDrawRect(renderer, &borderRect);
    
    // Draw empty background
    SDL_SetRenderDrawColor(renderer, emptyColor.r, emptyColor.g, emptyColor.b, emptyColor.a);
    SDL_Rect emptyRect = {x + 1, y + 1, width - 2, height - 2};
    SDL_RenderFillRect(renderer, &emptyRect);
    
    // Draw filled part
    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    int fillWidth = static_cast<int>((width - 2) * percentage / 100.0f);
    if (fillWidth > 0) {
        SDL_Rect fillRect = {x + 1, y + 1, fillWidth, height - 2};
        SDL_RenderFillRect(renderer, &fillRect);
    }
}

// Add store-related functions
void generateOffer(GameStateData& state);
void handleStoreInteraction(GameStateData& state, int x, int y);
void resetStoreState(GameStateData& state);

#endif // GAME_H 