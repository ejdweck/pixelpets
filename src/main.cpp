#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "game.h"
#include <ctime>
#include <random>
#include <algorithm>

// Structure to represent a raindrop
struct Raindrop {
    float x;
    float y;
    float speed;
    int length;
};

// Forward declarations for rendering functions
void drawPixelText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
SDL_Texture* createPlaceholderBackground(SDL_Renderer* renderer, const SDL_Color& bgColor, const std::string& label, int width, int height);
std::vector<Background> loadBackgrounds(SDL_Renderer* renderer);
void renderIntroScreen(SDL_Renderer* renderer, const ColorPalette& palette);
void renderNameEntryScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::string& name, bool inputActive);
void renderGenderSelectionScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Button& maleButton, const Button& femaleButton, Gender currentGender);
void renderStarterSelectionScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::vector<Plant>& plants, const std::vector<int>& starterIndices, const std::vector<Button>& starterButtons, int selectedIndex);
void renderPlantViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Plant& plant, const Player& player, const Button& menuButton, const Button& bgButton, BackgroundType bgType, const std::vector<Background>& backgrounds, const std::vector<Raindrop>& raindrops);
void renderMenuViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::vector<Plant>& plants, const Player& player, const std::vector<Button>& menuGridButtons, const Button& prevPageButton, const Button& nextPageButton, int currentPage, int plantsPerPage);
void renderGiftNotificationScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Plant& plant, const Button& okButton);
void renderCelebrationAnimation(SDL_Renderer* renderer, const ColorPalette& palette, 
                                const std::vector<Particle>& particles, const std::string& message);

// Define buttons as global variables to avoid redefinition
Button maleButton;
Button femaleButton;
Button menuButton;
Button bgButton;
Button continueButton;
Button prevPageButton;
Button nextPageButton;
Button okButton;
int newPlantIndex = -1;  // New plant index for gift notification
std::vector<Button> plantButtons(3);  // Plant selection buttons

// Add global variables for celebration
std::vector<Particle> celebrationParticles;
Uint32 celebrationStartTime = 0;

// Function to create a placeholder background texture
SDL_Texture* createPlaceholderBackground(SDL_Renderer* renderer, const SDL_Color& bgColor, const std::string& label, int width, int height) {
    // Create a texture that will be our render target
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                          SDL_TEXTUREACCESS_TARGET, width, height);
    
    if (!texture) {
        std::cerr << "Failed to create placeholder background texture!" << std::endl;
        return nullptr;
    }
    
    // Set the texture as the render target
    SDL_SetRenderTarget(renderer, texture);
    
    // Clear with the background color
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);
    
    // Draw different patterns based on the background type
    if (label == "Sunny") {
        // Draw sun
        int sunRadius = 20;
        int sunX = width / 4;
        int sunY = height / 5;
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow sun
        for (int y = -sunRadius; y <= sunRadius; y++) {
            for (int x = -sunRadius; x <= sunRadius; x++) {
                if (x*x + y*y <= sunRadius*sunRadius) {
                    SDL_RenderDrawPoint(renderer, sunX + x, sunY + y);
                }
            }
        }
        
        // Draw sun rays
        SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
        for (int i = 0; i < 8; i++) {
            double angle = i * M_PI / 4;
            int x1 = sunX + cos(angle) * sunRadius;
            int y1 = sunY + sin(angle) * sunRadius;
            int x2 = sunX + cos(angle) * (sunRadius + 10);
            int y2 = sunY + sin(angle) * (sunRadius + 10);
            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
    else if (label == "Rainy") {
        // We'll draw clouds, but not raindrops (they will be animated separately)
        
        // Draw clouds
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
        for (int cloud = 0; cloud < 3; cloud++) {
            int cloudX = 30 + cloud * 40;
            int cloudY = 30;
            int cloudRadius = 15;
            
            for (int i = 0; i < 3; i++) {
                for (int y = -cloudRadius; y <= cloudRadius; y++) {
                    for (int x = -cloudRadius; x <= cloudRadius; x++) {
                        if (x*x + y*y <= cloudRadius*cloudRadius) {
                            SDL_RenderDrawPoint(renderer, cloudX + x + i*10, cloudY + y);
                        }
                    }
                }
            }
        }
    }
    else if (label == "Cloudy") {
        // Draw clouds
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
        for (int cloud = 0; cloud < 5; cloud++) {
            int cloudX = 15 + cloud * 30;
            int cloudY = 20 + (cloud % 3) * 15;
            int cloudRadius = 10 + (cloud % 3) * 5;
            
            for (int i = 0; i < 3; i++) {
                for (int y = -cloudRadius; y <= cloudRadius; y++) {
                    for (int x = -cloudRadius; x <= cloudRadius; x++) {
                        if (x*x + y*y <= cloudRadius*cloudRadius) {
                            SDL_RenderDrawPoint(renderer, cloudX + x + i*8, cloudY + y);
                        }
                    }
                }
            }
        }
    }
    else if (label == "Night") {
        // Draw stars
        SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255);
        for (int i = 0; i < 50; i++) {
            int x = rand() % width;
            int y = rand() % height;
            SDL_RenderDrawPoint(renderer, x, y);
            
            // Some stars twinkle (bigger)
            if (i % 5 == 0) {
                SDL_RenderDrawPoint(renderer, x+1, y);
                SDL_RenderDrawPoint(renderer, x-1, y);
                SDL_RenderDrawPoint(renderer, x, y+1);
                SDL_RenderDrawPoint(renderer, x, y-1);
            }
        }
        
        // Draw moon
        int moonRadius = 15;
        int moonX = width - moonRadius - 10;
        int moonY = moonRadius + 10;
        
        SDL_SetRenderDrawColor(renderer, 230, 230, 200, 255);
        for (int y = -moonRadius; y <= moonRadius; y++) {
            for (int x = -moonRadius; x <= moonRadius; x++) {
                if (x*x + y*y <= moonRadius*moonRadius) {
                    SDL_RenderDrawPoint(renderer, moonX + x, moonY + y);
                }
            }
        }
    }
    
    // Draw background name at bottom of screen
    SDL_Rect textBg = {0, height - 20, width, 20};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &textBg);
    
    // Draw background name text
    drawPixelText(renderer, label, 10, height - 15, SDL_Color{255, 255, 255, 255});
    
    // Reset the render target back to the default
    SDL_SetRenderTarget(renderer, nullptr);
    
    return texture;
}

// Function to draw simple pixel text
void drawPixelText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    // Pixel font rendering with filled characters
    int charWidth = 6;
    int charHeight = 8;
    int spacing = 1;
    
    for (size_t i = 0; i < text.length(); i++) {
        char c = toupper(text[i]); // Convert to uppercase
        int charX = x + i * (charWidth + spacing);
        
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        
        // Define pixel patterns for each character
        // Each character is represented by a 5x7 bitmap pattern
        bool pixels[7][5] = {{0}};
        
        switch (c) {
            case 'A':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'B':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'C':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = true;
                pixels[3][0] = true;
                pixels[4][0] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'D':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'E':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][0] = true;
                pixels[2][0] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = true;
                pixels[5][0] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = pixels[6][4] = true;
                break;
            case 'F':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][0] = true;
                pixels[2][0] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = true;
                pixels[5][0] = true;
                pixels[6][0] = true;
                break;
            case 'G':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = true;
                pixels[3][0] = pixels[3][2] = pixels[3][3] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'H':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'I':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = true;
                pixels[1][1] = true;
                pixels[2][1] = true;
                pixels[3][1] = true;
                pixels[4][1] = true;
                pixels[5][1] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = true;
                break;
            case 'J':
                pixels[0][3] = pixels[0][4] = true;
                pixels[1][4] = true;
                pixels[2][4] = true;
                pixels[3][4] = true;
                pixels[4][1] = pixels[4][4] = true;
                pixels[5][1] = pixels[5][4] = true;
                pixels[6][2] = pixels[6][3] = true;
                break;
            case 'K':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][3] = true;
                pixels[2][0] = pixels[2][2] = true;
                pixels[3][0] = pixels[3][1] = true;
                pixels[4][0] = pixels[4][2] = true;
                pixels[5][0] = pixels[5][3] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'L':
                pixels[0][0] = true;
                pixels[1][0] = true;
                pixels[2][0] = true;
                pixels[3][0] = true;
                pixels[4][0] = true;
                pixels[5][0] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = pixels[6][4] = true;
                break;
            case 'M':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][1] = pixels[1][3] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][2] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][2] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'N':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][1] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][2] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][2] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][3] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'O':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'P':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = true;
                pixels[5][0] = true;
                pixels[6][0] = true;
                break;
            case 'Q':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][2] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][3] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][4] = true;
                break;
            case 'R':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = pixels[4][2] = true;
                pixels[5][0] = pixels[5][3] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'S':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][0] = true;
                pixels[2][0] = true;
                pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][4] = true;
                pixels[5][4] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'T':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][2] = true;
                pixels[2][2] = true;
                pixels[3][2] = true;
                pixels[4][2] = true;
                pixels[5][2] = true;
                pixels[6][2] = true;
                break;
            case 'U':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case 'V':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][1] = pixels[5][3] = true;
                pixels[6][2] = true;
                break;
            case 'W':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][2] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][2] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][2] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][3] = true;
                break;
            case 'X':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][1] = pixels[2][3] = true;
                pixels[3][2] = true;
                pixels[4][1] = pixels[4][3] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][0] = pixels[6][4] = true;
                break;
            case 'Y':
                pixels[0][0] = pixels[0][4] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][1] = pixels[2][3] = true;
                pixels[3][2] = true;
                pixels[4][2] = true;
                pixels[5][2] = true;
                pixels[6][2] = true;
                break;
            case 'Z':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][4] = true;
                pixels[2][3] = true;
                pixels[3][2] = true;
                pixels[4][1] = true;
                pixels[5][0] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = pixels[6][4] = true;
                break;
            case '0':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][3] = pixels[2][4] = true;
                pixels[3][0] = pixels[3][2] = pixels[3][4] = true;
                pixels[4][0] = pixels[4][1] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case '1':
                pixels[0][2] = true;
                pixels[1][1] = pixels[1][2] = true;
                pixels[2][0] = pixels[2][2] = true;
                pixels[3][2] = true;
                pixels[4][2] = true;
                pixels[5][2] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = pixels[6][4] = true;
                break;
            case '2':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][4] = true;
                pixels[3][3] = true;
                pixels[4][2] = true;
                pixels[5][1] = true;
                pixels[6][0] = pixels[6][1] = pixels[6][2] = pixels[6][3] = pixels[6][4] = true;
                break;
            case '3':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][4] = true;
                pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case '4':
                pixels[0][3] = true;
                pixels[1][2] = pixels[1][3] = true;
                pixels[2][1] = pixels[2][3] = true;
                pixels[3][0] = pixels[3][3] = true;
                pixels[4][0] = pixels[4][1] = pixels[4][2] = pixels[4][3] = pixels[4][4] = true;
                pixels[5][3] = true;
                pixels[6][3] = true;
                break;
            case '5':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][0] = true;
                pixels[2][0] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case '6':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = true;
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case '7':
                pixels[0][0] = pixels[0][1] = pixels[0][2] = pixels[0][3] = pixels[0][4] = true;
                pixels[1][4] = true;
                pixels[2][3] = true;
                pixels[3][2] = true;
                pixels[4][2] = true;
                pixels[5][2] = true;
                pixels[6][2] = true;
                break;
            case '8':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][1] = pixels[3][2] = pixels[3][3] = true;
                pixels[4][0] = pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case '9':
                pixels[0][1] = pixels[0][2] = pixels[0][3] = true;
                pixels[1][0] = pixels[1][4] = true;
                pixels[2][0] = pixels[2][4] = true;
                pixels[3][1] = pixels[3][2] = pixels[3][3] = pixels[3][4] = true;
                pixels[4][4] = true;
                pixels[5][0] = pixels[5][4] = true;
                pixels[6][1] = pixels[6][2] = pixels[6][3] = true;
                break;
            case ' ':
                // Space - do nothing
                break;
            case '-':
                pixels[3][0] = pixels[3][1] = pixels[3][2] = pixels[3][3] = pixels[3][4] = true;
                break;
            case ':':
                pixels[1][2] = true;
                pixels[5][2] = true;
                break;
            case '.':
                pixels[6][2] = true;
                break;
            default:
                // For any character we don't support, draw a filled box
                for (int row = 0; row < 7; row++) {
                    for (int col = 0; col < 5; col++) {
                        pixels[row][col] = true;
                    }
                }
                break;
        }
        
        // Draw the character based on pixel pattern
        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (pixels[row][col]) {
                    SDL_Rect pixel = {charX + col, y + row, 1, 1};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
    }
}

// Modify the loadPlants function to load all 60 plants
std::vector<Plant> loadPlants(SDL_Renderer* renderer) {
    std::vector<Plant> plants;
    const int TOTAL_PLANTS = 60; // Total number of plants available
    
    for (int i = 1; i <= TOTAL_PLANTS; i++) {
        Plant plant;
        plant.name = "Plant " + std::to_string(i);
        plant.filename = "assets/plant_" + std::to_string(i) + ".png";
        
        plant.texture = loadTexture(renderer, plant.filename);
        if (plant.texture == nullptr) {
            std::cerr << "Failed to load plant texture: " << plant.filename << std::endl;
            // Add placeholder if texture fails to load
            plant.width = 32;
            plant.height = 32;
        } else {
            SDL_QueryTexture(plant.texture, nullptr, nullptr, &plant.width, &plant.height);
        }
        
        // Assign a random preferred background to each plant
        plant.preferredBackground = static_cast<BackgroundType>(rand() % 4);
        
        plants.push_back(plant);
    }
    
    return plants;
}

// Update the main function to handle plant gifting and selection
int main(int argc, char* args[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "PixelPets - Plants",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Set render scale quality
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    
    // Create Player
    Player player;
    
    // Name input field
    bool nameInputActive = true;
    SDL_Rect nameInputRect = {SCREEN_WIDTH/2 - 50, SCREEN_HEIGHT/2, 100, 25};
    
    // Create starter plants (just use 3 from our existing assets)
    std::vector<Plant> starterPlants = {
        {"Bonsai", "assets/bonsai-pixel.png", nullptr, 0, 0},
        {"Cactus", "assets/tall_cactus_in_a_pot_with_flowers_on_each_branch__7c9bb967.png", nullptr, 0, 0},
        {"Bonsai2", "assets/generated-image-1742238370652.png", nullptr, 0, 0}
    };
    
    // Load backgrounds
    std::vector<Background> backgrounds = loadBackgrounds(renderer);
    BackgroundType currentBackground = BackgroundType::SUNNY;
    
    // Load plants
    std::vector<Plant> plants = loadPlants(renderer);
    
    // Create player data
    Player playerData;
    
    // Set initial game state
    GameState gameState = GameState::INTRO;
    
    // Text input variables for name entry
    std::string inputText = "";
    bool inputActive = true;
    
    // Gender selection variables
    maleButton = {{SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT / 2 - 25, 50, 50}, false};
    femaleButton = {{3 * SCREEN_WIDTH / 4 - 25, SCREEN_HEIGHT / 2 - 25, 50, 50}, false};
    
    // Plant view state variables
    menuButton = {{SCREEN_WIDTH - MENU_BUTTON_SIZE - 5, 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false}; // Top right corner
    bgButton = {{5, 5, BG_BUTTON_SIZE, BG_BUTTON_SIZE}, false}; // Top left corner
    
    // For continue button that appears in multiple screens
    continueButton = {{SCREEN_WIDTH/2 - 30, SCREEN_HEIGHT - 50, 60, 30}, false};
    
    // Menu view navigation buttons
    prevPageButton = {{10, SCREEN_HEIGHT - 30, 50, 20}, false};
    nextPageButton = {{SCREEN_WIDTH - 60, SCREEN_HEIGHT - 30, 50, 20}, false};
    
    // Gift notification button
    okButton = {{SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT * 3 / 4, 50, 20}, false};
    
    // Starter plant selection variables
    std::vector<int> starterIndices = getRandomIndices(0, plants.size() - 1, 3);
    std::vector<Button> starterButtons;
    int buttonWidth = (SCREEN_WIDTH - 40) / 3;
    int buttonHeight = buttonWidth;
    
    for (int i = 0; i < 3; i++) {
        int x = 20 + i * buttonWidth;
        Button button = {{x, SCREEN_HEIGHT / 2 - buttonHeight / 2, buttonWidth - 10, buttonHeight}, false};
        starterButtons.push_back(button);
    }
    
    // Menu view state variables
    int currentPage = 0;
    int plantsPerPage = MENU_GRID_ROWS * MENU_GRID_COLS;
    std::vector<Button> menuGridButtons;
    
    // Update menu grid buttons based on the current page
    auto updateMenuGrid = [&]() {
        menuGridButtons.clear();
        int itemWidth = (SCREEN_WIDTH - (MENU_GRID_COLS + 1) * MENU_ITEM_PADDING) / MENU_GRID_COLS;
        int itemHeight = itemWidth;
        
        for (int row = 0; row < MENU_GRID_ROWS; row++) {
            for (int col = 0; col < MENU_GRID_COLS; col++) {
                int index = currentPage * plantsPerPage + row * MENU_GRID_COLS + col;
                if (index < plants.size()) {
                    int x = MENU_ITEM_PADDING + col * (itemWidth + MENU_ITEM_PADDING);
                    int y = MENU_ITEM_PADDING + row * (itemHeight + MENU_ITEM_PADDING);
                    Button button = {{x, y, itemWidth, itemHeight}, false};
                    menuGridButtons.push_back(button);
                }
            }
        }
    };
    
    // Initialize the menu grid
    updateMenuGrid();
    
    // Setup raindrops for animation
    const int MAX_RAINDROPS = 100;
    std::vector<Raindrop> raindrops(MAX_RAINDROPS);
    
    // Initialize raindrops
    for (int i = 0; i < MAX_RAINDROPS; i++) {
        raindrops[i].x = rand() % SCREEN_WIDTH;
        raindrops[i].y = rand() % SCREEN_HEIGHT;
        raindrops[i].speed = 2.0f + (rand() % 20) / 10.0f; // Speed between 2.0 and 4.0
        raindrops[i].length = 5 + rand() % 10; // Length between 5 and 14 pixels
    }
    
    // Main loop flag
    bool quit = false;
    
    // Event handler
    SDL_Event e;
    
    // Color palette
    ColorPalette palette;
    
    // Enable text input
    SDL_StartTextInput();
    
    // For blinking cursor
    Uint32 cursorTimer = 0;
    bool showCursor = true;
    
    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            // User requests quit
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // User presses a key
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
                else if (gameState == GameState::NAME_ENTRY) {
                    if (e.key.keysym.sym == SDLK_BACKSPACE && player.name.length() > 0) {
                        player.name.pop_back();
                    }
                    else if (e.key.keysym.sym == SDLK_RETURN) {
                        if (player.name.length() > 0) {
                            gameState = GameState::GENDER_SELECTION;
                        }
                    }
                }
            }
            // Text input handling
            else if (e.type == SDL_TEXTINPUT && gameState == GameState::NAME_ENTRY) {
                if (player.name.length() < 10) {
                    player.name += e.text.text;
                }
            }
            // Handle mouse/touch events
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                
                std::cout << "Touch at: " << mouseX << ", " << mouseY << std::endl;
                
                if (gameState == GameState::INTRO) {
                    gameState = GameState::NAME_ENTRY;
                }
                else if (gameState == GameState::NAME_ENTRY) {
                    if (continueButton.contains(mouseX, mouseY) && player.name.length() > 0) {
                        gameState = GameState::GENDER_SELECTION;
                    }
                }
                else if (gameState == GameState::GENDER_SELECTION) {
                    if (maleButton.contains(mouseX, mouseY)) {
                        player.gender = Gender::MALE;
                    }
                    else if (femaleButton.contains(mouseX, mouseY)) {
                        player.gender = Gender::FEMALE;
                    }
                    else if (continueButton.contains(mouseX, mouseY)) {
                        gameState = GameState::STARTER_SELECTION;
                        
                        // Calculate plant selection button positions
                        int buttonWidth = SCREEN_WIDTH / 3 - 10;
                        int buttonHeight = SCREEN_HEIGHT / 2;
                        int buttonY = SCREEN_HEIGHT / 4;
                        
                        for (int i = 0; i < 3; i++) {
                            plantButtons[i].rect = {
                                5 + i * (buttonWidth + 5),
                                buttonY,
                                buttonWidth,
                                buttonHeight
                            };
                            plantButtons[i].isPressed = false;
                        }
                    }
                }
                else if (gameState == GameState::STARTER_SELECTION) {
                    for (int i = 0; i < 3; i++) {
                        if (plantButtons[i].contains(mouseX, mouseY)) {
                            player.selectedPlantIndex = starterIndices[i];
                            break;
                        }
                    }
                    
                    if (continueButton.contains(mouseX, mouseY) && player.selectedPlantIndex >= 0) {
                        player.ownedPlants.push_back(player.selectedPlantIndex);
                        currentBackground = plants[player.selectedPlantIndex].preferredBackground;
                        gameState = GameState::PLANT_VIEW;
                    }
                }
                else if (gameState == GameState::PLANT_VIEW) {
                    if (menuButton.contains(mouseX, mouseY)) {
                        gameState = GameState::MENU_VIEW;
                    }
                    else if (bgButton.contains(mouseX, mouseY)) {
                        int randomBg = rand() % 4;
                        currentBackground = static_cast<BackgroundType>(randomBg);
                        
                        plants[player.selectedPlantIndex].preferredBackground = currentBackground;
                    }
                }
                else if (gameState == GameState::MENU_VIEW) {
                    for (int i = 0; i < menuGridButtons.size(); i++) {
                        int plantIndex = currentPage * plantsPerPage + i;
                        if (plantIndex < plants.size() && menuGridButtons[i].contains(mouseX, mouseY)) {
                            if (std::find(player.ownedPlants.begin(), player.ownedPlants.end(), plantIndex) != player.ownedPlants.end()) {
                                player.selectedPlantIndex = plantIndex;
                                currentBackground = plants[plantIndex].preferredBackground;
                                gameState = GameState::PLANT_VIEW;
                                break;
                            }
                        }
                    }
                    
                    if (prevPageButton.contains(mouseX, mouseY) && currentPage > 0) {
                        currentPage--;
                        updateMenuGrid();
                    } else if (nextPageButton.contains(mouseX, mouseY) && (currentPage + 1) * plantsPerPage < plants.size()) {
                        currentPage++;
                        updateMenuGrid();
                    }
                    
                    if (mouseX < 0 || mouseY < 0 || mouseX > SCREEN_WIDTH || mouseY > SCREEN_HEIGHT) {
                        gameState = GameState::PLANT_VIEW;
                    }
                }
                else if (gameState == GameState::CELEBRATION_ANIMATION) {
                    // Any click during celebration will move to gift notification
                    gameState = GameState::GIFT_NOTIFICATION;
                }
                else if (gameState == GameState::GIFT_NOTIFICATION) {
                    // Handle OK button click
                    if (okButton.contains(mouseX, mouseY)) {
                        // Return to plant view
                        gameState = GameState::PLANT_VIEW;
                        // Optionally switch to the new plant
                        player.selectedPlantIndex = newPlantIndex;
                        currentBackground = plants[newPlantIndex].preferredBackground;
                    }
                }
            }
        }
        
        // Update blinking cursor
        if (SDL_GetTicks() - cursorTimer > 500) {
            showCursor = !showCursor;
            cursorTimer = SDL_GetTicks();
        }
        
        // Update raindrops
        if (gameState == GameState::PLANT_VIEW && 
            plants[player.selectedPlantIndex].preferredBackground == BackgroundType::RAINY) {
            for (int i = 0; i < MAX_RAINDROPS; i++) {
                raindrops[i].y += raindrops[i].speed;
                
                if (raindrops[i].y > SCREEN_HEIGHT) {
                    raindrops[i].x = rand() % SCREEN_WIDTH;
                    raindrops[i].y = -raindrops[i].length;
                    raindrops[i].speed = 2.0f + (rand() % 20) / 10.0f;
                }
            }
        }
        
        // Check if it's time to gift a new plant
        Uint32 currentTime = SDL_GetTicks();
        if (gameState == GameState::PLANT_VIEW && 
            player.ownedPlants.size() < plants.size() && 
            currentTime - player.lastGiftTime >= PLANT_GIFT_INTERVAL) {
            
            // Find a plant the player doesn't already own
            std::vector<int> availablePlants;
            for (int i = 0; i < plants.size(); i++) {
                // Skip if player already owns this plant
                if (std::find(player.ownedPlants.begin(), player.ownedPlants.end(), i) != player.ownedPlants.end()) {
                    continue;
                }
                availablePlants.push_back(i);
            }
            
            if (!availablePlants.empty()) {
                // Randomly select one of the available plants
                newPlantIndex = availablePlants[rand() % availablePlants.size()];
                player.ownedPlants.push_back(newPlantIndex);
                player.lastGiftTime = currentTime;
                
                // Initialize celebration particles
                celebrationParticles.clear();
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    celebrationParticles.push_back(createRandomParticle(SCREEN_WIDTH, SCREEN_HEIGHT));
                }
                
                // Change to celebration animation state
                celebrationStartTime = currentTime;
                gameState = GameState::CELEBRATION_ANIMATION;
            }
        }
        
        // Clear screen with GameBoy-inspired background color
        SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
        SDL_RenderClear(renderer);
        
        // Update celebration particles if in CELEBRATION_ANIMATION state
        if (gameState == GameState::CELEBRATION_ANIMATION) {
            // Check if celebration duration has elapsed
            if (currentTime - celebrationStartTime > CELEBRATION_DURATION) {
                gameState = GameState::GIFT_NOTIFICATION;
            }
            
            // Update particle positions and lifespans
            for (auto& particle : celebrationParticles) {
                // Update position
                particle.x += particle.velocityX;
                particle.y += particle.velocityY;
                
                // Add gravity effect
                particle.velocityY += 0.05f;
                
                // Increase age
                particle.age++;
                
                // If particle is too old, reset it
                if (particle.age >= particle.lifespan) {
                    particle = createRandomParticle(SCREEN_WIDTH, SCREEN_HEIGHT);
                }
            }
        }
        
        // Render based on current state
        switch (gameState) {
            case GameState::INTRO:
                renderIntroScreen(renderer, palette);
                break;
                
            case GameState::NAME_ENTRY:
                renderNameEntryScreen(renderer, palette, player.name, inputActive);
                break;
                
            case GameState::GENDER_SELECTION:
                renderGenderSelectionScreen(renderer, palette, maleButton, femaleButton, player.gender);
                break;
                
            case GameState::STARTER_SELECTION:
                renderStarterSelectionScreen(renderer, palette, plants, starterIndices, starterButtons, player.selectedPlantIndex);
                break;
                
            case GameState::PLANT_VIEW:
                renderPlantViewScreen(renderer, palette, plants[player.selectedPlantIndex], player, menuButton, bgButton, currentBackground, backgrounds, raindrops);
                break;
                
            case GameState::MENU_VIEW:
                renderMenuViewScreen(renderer, palette, plants, player, menuGridButtons, prevPageButton, nextPageButton, currentPage, plantsPerPage);
                break;
                
            case GameState::GIFT_NOTIFICATION:
                renderGiftNotificationScreen(renderer, palette, plants[newPlantIndex], okButton);
                break;
                
            case GameState::CELEBRATION_ANIMATION:
                renderCelebrationAnimation(renderer, palette, celebrationParticles, "CONGRATULATIONS!");
                break;
        }
        
        // Always draw the menu button (only in PLANT_VIEW and MENU_VIEW)
        if (gameState == GameState::PLANT_VIEW || gameState == GameState::MENU_VIEW) {
            drawButton(renderer, menuButton, palette);
        }
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Add a small delay to reduce CPU usage
        SDL_Delay(16);
    }
    
    // Disable text input
    SDL_StopTextInput();
    
    // Free resources
    for (auto& plant : plants) {
        if (plant.texture) {
            SDL_DestroyTexture(plant.texture);
        }
    }
    
    for (auto& plant : starterPlants) {
        if (plant.texture && std::find_if(plants.begin(), plants.end(), 
                [&plant](const Plant& p) { return p.texture == plant.texture; }) == plants.end()) {
            SDL_DestroyTexture(plant.texture);
        }
    }
    
    // Free background textures
    for (auto& bg : backgrounds) {
        if (bg.texture) {
            SDL_DestroyTexture(bg.texture);
        }
    }
    
    // Destroy renderer
    SDL_DestroyRenderer(renderer);
    
    // Destroy window
    SDL_DestroyWindow(window);
    
    // Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}

// Add rendering functions for different game states
void renderIntroScreen(SDL_Renderer* renderer, const ColorPalette& palette) {
    // Draw title screen
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_Rect titleRect = {10, 40, SCREEN_WIDTH - 20, 60};
    SDL_RenderFillRect(renderer, &titleRect);
    
    // Draw title text
    drawPixelText(renderer, "PIXELPETS", SCREEN_WIDTH/2 - 30, 50, palette.white);
    drawPixelText(renderer, "PLANTS", SCREEN_WIDTH/2 - 20, 70, palette.white);
    
    // Draw pixel art plant
    int plantX = SCREEN_WIDTH / 2;
    int plantY = SCREEN_HEIGHT / 2;
    int plantRadius = 20;
    
    // Draw pot
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown
    SDL_Rect pot = {plantX - 15, plantY + 10, 30, 20};
    SDL_RenderFillRect(renderer, &pot);
    
    // Draw stem
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255); // Dark green
    SDL_Rect stem = {plantX - 2, plantY - 30, 4, 40};
    SDL_RenderFillRect(renderer, &stem);
    
    // Draw leaves
    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255); // Green
    for (int i = 0; i < 3; i++) {
        SDL_Rect leaf = {plantX + (i-1)*10 - 5, plantY - 30 + i*10, 10, 5};
        SDL_RenderFillRect(renderer, &leaf);
    }
    
    // Draw instruction text
    drawPixelText(renderer, "TAP TO START", SCREEN_WIDTH/2 - 40, SCREEN_HEIGHT - 40, palette.white);
    
    // Draw blinking effect
    if (SDL_GetTicks() / 500 % 2 == 0) {
        SDL_Rect highlight = {SCREEN_WIDTH/2 - 42, SCREEN_HEIGHT - 42, 84, 14};
        SDL_SetRenderDrawColor(renderer, palette.lightest.r, palette.lightest.g, palette.lightest.b, 100);
        SDL_RenderDrawRect(renderer, &highlight);
    }
}

void renderNameEntryScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::string& name, bool inputActive) {
    // Draw title
    drawPixelText(renderer, "ENTER YOUR NAME:", SCREEN_WIDTH/2 - 50, 30, palette.white);
    
    // Create input rectangle
    SDL_Rect nameInputRect = {SCREEN_WIDTH/2 - 60, SCREEN_HEIGHT/2 - 15, 120, 30};
    
    // Draw input field
    drawTextInputField(renderer, name, nameInputRect.x, nameInputRect.y, nameInputRect.w, nameInputRect.h, palette, inputActive);
    
    // Draw input text in the field
    drawPixelText(renderer, name, nameInputRect.x + 5, nameInputRect.y + 8, palette.black);
    
    // Draw blinking cursor
    static bool showCursor = true;
    if (SDL_GetTicks() / 500 % 2 == 0) {
        showCursor = !showCursor;
    }
    
    if (showCursor && inputActive) {
        int cursorX = nameInputRect.x + 5 + name.length() * 6; // Approximate width of each character
        SDL_Rect cursor = {cursorX, nameInputRect.y + 5, 2, 15};
        SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
        SDL_RenderFillRect(renderer, &cursor);
    }
    
    // Draw continue button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &continueButton.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    SDL_RenderDrawRect(renderer, &continueButton.rect);
    
    // Draw button text
    drawPixelText(renderer, "NEXT", continueButton.rect.x + 15, continueButton.rect.y + 10, palette.white);
}

void renderGenderSelectionScreen(SDL_Renderer* renderer, const ColorPalette& palette,
                                 const Button& maleButton, const Button& femaleButton, Gender currentGender) {
    // Draw title
    drawPixelText(renderer, "SELECT YOUR GENDER:", SCREEN_WIDTH/2 - 60, 30, palette.white);
    
    // Draw gender buttons
    drawGenderButton(renderer, maleButton, true, currentGender == Gender::MALE, palette);
    drawGenderButton(renderer, femaleButton, false, currentGender == Gender::FEMALE, palette);
    
    // Draw labels
    drawPixelText(renderer, "MALE", maleButton.rect.x + 10, maleButton.rect.y + maleButton.rect.h + 5, palette.white);
    drawPixelText(renderer, "FEMALE", femaleButton.rect.x + 5, femaleButton.rect.y + femaleButton.rect.h + 5, palette.white);
    
    // Draw continue button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &continueButton.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    SDL_RenderDrawRect(renderer, &continueButton.rect);
    
    // Draw button text
    drawPixelText(renderer, "NEXT", continueButton.rect.x + 15, continueButton.rect.y + 10, palette.white);
}

void renderStarterSelectionScreen(SDL_Renderer* renderer, const ColorPalette& palette,
                                 const std::vector<Plant>& plants, const std::vector<int>& starterIndices,
                                 const std::vector<Button>& starterButtons, int selectedIndex) {
    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
    SDL_RenderClear(renderer);
    
    // Draw title
    std::string title = "CHOOSE YOUR STARTER";
    int titleWidth = title.length() * 6;
    int titleX = (SCREEN_WIDTH - titleWidth) / 2;
    
    drawPixelText(renderer, title, titleX, 30, palette.white);
    
    // Draw starter buttons
    for (int i = 0; i < starterButtons.size(); i++) {
        int plantIndex = starterIndices[i];
        bool isSelected = (plantIndex == selectedIndex);
        
        drawPlantSelectionButton(renderer, starterButtons[i], plants[plantIndex].texture, 
                                plants[plantIndex].width, plants[plantIndex].height, 
                                isSelected, palette);
        
        // Draw plant name below the button
        std::string name = plants[plantIndex].name;
        int nameWidth = name.length() * 6;
        int nameX = starterButtons[i].rect.x + (starterButtons[i].rect.w - nameWidth) / 2;
        int nameY = starterButtons[i].rect.y + starterButtons[i].rect.h + 5;
        
        drawPixelText(renderer, name, nameX, nameY, palette.white);
    }
    
    // Draw continue button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &continueButton.rect);
    
    // Draw button border
    SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
    SDL_RenderDrawRect(renderer, &continueButton.rect);
    
    // Draw button text
    drawPixelText(renderer, "START", continueButton.rect.x + 15, continueButton.rect.y + 10, palette.white);
    
    // Draw instruction
    std::string instruction = "Tap to select";
    int instrWidth = instruction.length() * 6;
    int instrX = (SCREEN_WIDTH - instrWidth) / 2;
    
    drawPixelText(renderer, instruction, instrX, SCREEN_HEIGHT - 30, palette.white);
}

void renderPlantViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Plant& plant,
                           const Player& player, const Button& menuButton, const Button& bgButton,
                           BackgroundType bgType, const std::vector<Background>& backgrounds,
                           const std::vector<Raindrop>& raindrops) {
    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
    SDL_RenderClear(renderer);
    
    // Draw background
    int bgIndex = static_cast<int>(bgType);
    if (bgIndex >= 0 && bgIndex < static_cast<int>(backgrounds.size()) && backgrounds[bgIndex].texture) {
        // Scale to fit screen
        renderTexture(renderer, backgrounds[bgIndex].texture, 0, 0, nullptr, 
                     (double)SCREEN_WIDTH / backgrounds[bgIndex].width);
    }
    
    // Draw raindrops if rainy background
    if (bgType == BackgroundType::RAINY) {
        SDL_SetRenderDrawColor(renderer, 173, 216, 230, 150); // Light blue, semi-transparent
        for (const auto& drop : raindrops) {
            SDL_RenderDrawLine(renderer, 
                              drop.x, drop.y, 
                              drop.x, drop.y + drop.length);
        }
    }
    
    // Draw plant
    if (plant.texture) {
        // Calculate scaling to make the plant 90% of screen height and width
        double scaleHeight = (SCREEN_HEIGHT * 0.9) / plant.height;
        double scaleWidth = (SCREEN_WIDTH * 0.9) / plant.width;
        double scale = std::min(scaleHeight, scaleWidth);
        
        // Apply additional scaling factor to make the plant larger
        scale *= 1.25;
        
        int scaledWidth = plant.width * scale;
        int scaledHeight = plant.height * scale;
        int x = (SCREEN_WIDTH - scaledWidth) / 2;
        int y = (SCREEN_HEIGHT - scaledHeight) / 2;
        
        renderTexture(renderer, plant.texture, x, y, nullptr, scale);
    }
    
    // Draw player info
    std::string playerInfo = player.name;
    int textWidth = playerInfo.length() * 6; // Approximate width based on font size
    
    // Position the player info to not overlap with background button
    SDL_Rect playerInfoBg = {BG_BUTTON_SIZE + 10, 5, textWidth + 5, 15};
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &playerInfoBg);
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &playerInfoBg);
    
    drawPixelText(renderer, playerInfo, playerInfoBg.x + 3, playerInfoBg.y + 3, palette.white);
    
    // Draw background info
    std::string bgInfo = "Weather: " + BackgroundTypeNames[static_cast<int>(bgType)];
    textWidth = bgInfo.length() * 6;
    
    SDL_Rect bgInfoBg = {SCREEN_WIDTH - textWidth - 10, SCREEN_HEIGHT - 20, textWidth + 5, 15};
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &bgInfoBg);
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &bgInfoBg);
    
    drawPixelText(renderer, bgInfo, bgInfoBg.x + 3, bgInfoBg.y + 3, palette.white);
    
    // Draw menu button
    drawButton(renderer, menuButton, palette);
    
    // Draw background cycle button
    drawBgButton(renderer, bgButton, palette);
    
    // Draw plant collection count
    std::string collectionInfo = "Collection: " + std::to_string(player.ownedPlants.size()) + "/" + std::to_string(60);
    textWidth = collectionInfo.length() * 6;
    
    SDL_Rect collectionBg = {SCREEN_WIDTH - textWidth - 10, 5, textWidth + 5, 15};
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &collectionBg);
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &collectionBg);
    
    drawPixelText(renderer, collectionInfo, collectionBg.x + 3, collectionBg.y + 3, palette.white);
}

void renderMenuViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                         const std::vector<Plant>& plants, const Player& player,
                         const std::vector<Button>& menuGridButtons, 
                         const Button& prevPageButton, const Button& nextPageButton,
                         int currentPage, int plantsPerPage) {
    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
    SDL_RenderClear(renderer);
    
    // Draw title
    std::string title = "PLANT COLLECTION";
    int titleWidth = title.length() * 6;
    int titleX = (SCREEN_WIDTH - titleWidth) / 2;
    
    drawPixelText(renderer, title, titleX, 5, palette.white);
    
    // Draw grid
    for (int i = 0; i < menuGridButtons.size(); i++) {
        int plantIndex = currentPage * plantsPerPage + i;
        if (plantIndex < plants.size()) {
            bool isOwned = std::find(player.ownedPlants.begin(), player.ownedPlants.end(), plantIndex) != player.ownedPlants.end();
            bool isSelected = (plantIndex == player.selectedPlantIndex);
            
            // Draw button with plant texture if owned
            if (isOwned) {
                drawPlantSelectionButton(renderer, menuGridButtons[i], plants[plantIndex].texture, 
                                         plants[plantIndex].width, plants[plantIndex].height, 
                                         isSelected, palette);
            } 
            // Draw locked placeholder if not owned
            else {
                SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
                SDL_RenderFillRect(renderer, &menuGridButtons[i].rect);
                
                SDL_SetRenderDrawColor(renderer, palette.black.r, palette.black.g, palette.black.b, palette.black.a);
                SDL_RenderDrawRect(renderer, &menuGridButtons[i].rect);
                
                // Draw lock icon
                int centerX = menuGridButtons[i].rect.x + menuGridButtons[i].rect.w / 2;
                int centerY = menuGridButtons[i].rect.y + menuGridButtons[i].rect.h / 2;
                
                // Draw lock body
                SDL_Rect lockBody = {centerX - 5, centerY - 3, 10, 8};
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                SDL_RenderFillRect(renderer, &lockBody);
                
                // Draw lock shackle
                SDL_Rect lockShackle = {centerX - 3, centerY - 9, 6, 6};
                SDL_RenderDrawRect(renderer, &lockShackle);
            }
        }
    }
    
    // Draw navigation buttons
    drawNavButtons(renderer, prevPageButton, nextPageButton, palette);
    
    // Draw page indicator
    int totalPages = (plants.size() + plantsPerPage - 1) / plantsPerPage;
    std::string pageInfo = "Page " + std::to_string(currentPage + 1) + "/" + std::to_string(totalPages);
    
    int pageInfoWidth = pageInfo.length() * 6;
    int pageInfoX = (SCREEN_WIDTH - pageInfoWidth) / 2;
    
    drawPixelText(renderer, pageInfo, pageInfoX, SCREEN_HEIGHT - 25, palette.white);
    
    // Draw collection info
    std::string collectionInfo = "Collected: " + std::to_string(player.ownedPlants.size()) + "/" + std::to_string(plants.size());
    int collectionInfoWidth = collectionInfo.length() * 6;
    int collectionInfoX = (SCREEN_WIDTH - collectionInfoWidth) / 2;
    
    drawPixelText(renderer, collectionInfo, collectionInfoX, SCREEN_HEIGHT - 15, palette.white);
}

void renderGiftNotificationScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                                 const Plant& plant, const Button& okButton) {
    // Draw semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // Draw notification box
    SDL_Rect notificationRect = {SCREEN_WIDTH / 6, SCREEN_HEIGHT / 4, 2 * SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2};
    drawNotificationBox(renderer, "NEW PLANT!", "You received a new plant!", notificationRect, palette);
    
    // Draw title
    std::string title = "NEW PLANT!";
    int titleWidth = title.length() * 6;
    int titleX = (SCREEN_WIDTH - titleWidth) / 2;
    
    drawPixelText(renderer, title, titleX, notificationRect.y + 10, palette.yellow);
    
    // Draw plant name
    std::string plantName = plant.name;
    int nameWidth = plantName.length() * 6;
    int nameX = (SCREEN_WIDTH - nameWidth) / 2;
    
    drawPixelText(renderer, plantName, nameX, notificationRect.y + 30, palette.white);
    
    // Draw plant image
    if (plant.texture) {
        // Scale to fit notification box
        double scaleHeight = (notificationRect.h * 0.5) / plant.height;
        double scaleWidth = (notificationRect.w * 0.7) / plant.width;
        double scale = std::min(scaleHeight, scaleWidth);
        
        int scaledWidth = plant.width * scale;
        int scaledHeight = plant.height * scale;
        int x = (SCREEN_WIDTH - scaledWidth) / 2;
        int y = notificationRect.y + 45;
        
        renderTexture(renderer, plant.texture, x, y, nullptr, scale);
    }
    
    // Draw OK button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &okButton.rect);
    
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &okButton.rect);
    
    // Draw OK text
    std::string okText = "OK";
    int okWidth = okText.length() * 6;
    int okX = okButton.rect.x + (okButton.rect.w - okWidth) / 2;
    int okY = okButton.rect.y + (okButton.rect.h - 7) / 2;
    
    drawPixelText(renderer, okText, okX, okY, palette.white);
}

// Add function to load backgrounds
std::vector<Background> loadBackgrounds(SDL_Renderer* renderer) {
    // Create placeholder backgrounds
    struct BgColor {
        std::string name;
        SDL_Color color;
    };
    
    BgColor bgColors[] = {
        {"Sunny", SDL_Color{135, 206, 235, 255}},    // sky blue
        {"Rainy", SDL_Color{105, 105, 105, 255}},    // dark gray
        {"Cloudy", SDL_Color{176, 196, 222, 255}},   // light steel blue
        {"Night", SDL_Color{25, 25, 112, 255}}       // midnight blue
    };

    std::vector<Background> backgrounds;
    for (int i = 0; i < 4; i++) {
        Background bg;
        bg.name = bgColors[i].name;
        bg.filename = ""; // No file to load, we're creating it programmatically
        bg.texture = createPlaceholderBackground(renderer, bgColors[i].color, bgColors[i].name, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (bg.texture) {
            bg.width = SCREEN_WIDTH;
            bg.height = SCREEN_HEIGHT;
            backgrounds.push_back(bg);
        }
    }
    
    return backgrounds;
}

// Add the implementation of the celebration animation rendering function
void renderCelebrationAnimation(SDL_Renderer* renderer, const ColorPalette& palette, 
                                const std::vector<Particle>& particles, const std::string& message) {
    // Clear screen with dark background color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Draw all particles
    for (const auto& particle : particles) {
        drawParticle(renderer, particle);
    }
    
    // Draw celebration message
    int messageWidth = message.length() * 6;
    int messageX = (SCREEN_WIDTH - messageWidth) / 2;
    int messageY = SCREEN_HEIGHT / 4;
    
    // Draw message with rainbow effect
    for (size_t i = 0; i < message.length(); i++) {
        SDL_Color charColor;
        int hue = (SDL_GetTicks() / 10 + i * 20) % 360;
        
        // Convert HSV to RGB (simplified)
        if (hue < 60) {
            charColor = {255, static_cast<Uint8>(hue * 255 / 60), 0, 255};
        } else if (hue < 120) {
            charColor = {static_cast<Uint8>((120 - hue) * 255 / 60), 255, 0, 255};
        } else if (hue < 180) {
            charColor = {0, 255, static_cast<Uint8>((hue - 120) * 255 / 60), 255};
        } else if (hue < 240) {
            charColor = {0, static_cast<Uint8>((240 - hue) * 255 / 60), 255, 255};
        } else if (hue < 300) {
            charColor = {static_cast<Uint8>((hue - 240) * 255 / 60), 0, 255, 255};
        } else {
            charColor = {255, 0, static_cast<Uint8>((360 - hue) * 255 / 60), 255};
        }
        
        // Draw single character with its color
        std::string singleChar(1, message[i]);
        drawPixelText(renderer, singleChar, messageX + i * 6, messageY, charColor);
    }
    
    // Draw a larger sparkling plant emoji in the center
    std::string plantEmoji = "🌱";
    int emojiX = SCREEN_WIDTH / 2 - 15;
    int emojiY = SCREEN_HEIGHT / 2 - 15;
    
    // Draw stylized plant instead of emoji (which can't be rendered in pixel font)
    int plantX = SCREEN_WIDTH / 2;
    int plantY = SCREEN_HEIGHT / 2;
    
    // Draw pot
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown
    SDL_Rect pot = {plantX - 15, plantY + 10, 30, 20};
    SDL_RenderFillRect(renderer, &pot);
    
    // Draw stem with shimmer effect
    int shimmer = (SDL_GetTicks() / 100) % 255;
    SDL_SetRenderDrawColor(renderer, 0, 200 + shimmer/4, 0, 255); // Shimmering green
    SDL_Rect stem = {plantX - 2, plantY - 30, 4, 40};
    SDL_RenderFillRect(renderer, &stem);
    
    // Draw leaves with shimmer effect
    SDL_SetRenderDrawColor(renderer, 100, 255 - shimmer/4, 100, 255); // Shimmering light green
    for (int i = 0; i < 3; i++) {
        SDL_Rect leaf = {plantX + (i-1)*10 - 5, plantY - 30 + i*10, 10, 5};
        SDL_RenderFillRect(renderer, &leaf);
    }
    
    // Draw a hint at the bottom
    std::string hintText = "NEW PLANT ACQUIRED!";
    int hintWidth = hintText.length() * 6;
    int hintX = (SCREEN_WIDTH - hintWidth) / 2;
    int hintY = SCREEN_HEIGHT * 3 / 4;
    
    // Draw with pulsing effect
    int modValue = (SDL_GetTicks() / 10) % 200 - 100;
    int pulse = modValue < 0 ? -modValue : modValue; // Manually calculate absolute value
    SDL_Color pulseColor = {255, 255, static_cast<Uint8>(100 + pulse), 255};
    drawPixelText(renderer, hintText, hintX, hintY, pulseColor);
} 