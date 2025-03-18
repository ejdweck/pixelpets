#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include "../include/game.h"
#include "../include/render.h"

// Global font
TTF_Font* gFont = nullptr;

// Initialize SDL_ttf and load font
bool initFont() {
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    
    // Try different font sizes in case the file is not found
    const int fontSizes[] = {8, 10, 12, 14, 16};
    for (int size : fontSizes) {
        gFont = TTF_OpenFont("assets/fonts/pixel.ttf", size);
        if (gFont != nullptr) {
            std::cout << "Successfully loaded font at size " << size << std::endl;
            return true;
        }
    }
    
    std::cerr << "Failed to load font at any size! SDL_ttf Error: " << TTF_GetError() << std::endl;
    return false;
}

// Clean up font
void cleanupFont() {
    if (gFont != nullptr) {
        TTF_CloseFont(gFont);
        gFont = nullptr;
    }
    TTF_Quit();
}

// Helper function to get text dimensions
SDL_Rect getTextDimensions(const std::string& text) {
    SDL_Rect dimensions = {0, 0, 0, 0};
    if (!gFont) return dimensions;
    
    int w, h;
    if (TTF_SizeText(gFont, text.c_str(), &w, &h) == 0) {
        dimensions.w = w;
        dimensions.h = h;
    }
    return dimensions;
}

// Helper function to center text horizontally
int centerTextX(const std::string& text, int containerWidth) {
    SDL_Rect dimensions = getTextDimensions(text);
    return (containerWidth - dimensions.w) / 2;
}

// New function to render text using SDL_ttf
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (!gFont) return;
    
    // Enable alpha blending for text
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, text.c_str(), color);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    }
    
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Forward declarations
void drawPixelText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, SDL_Rect* clip, double scale);

// Function to create a placeholder background texture
SDL_Texture* createPlaceholderBackground(SDL_Renderer* renderer, const SDL_Color& bgColor, const std::string& label, int width, int height) {
    if (!renderer) {
        std::cerr << "Invalid renderer in createPlaceholderBackground" << std::endl;
        return nullptr;
    }
    
    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid dimensions in createPlaceholderBackground: " << width << "x" << height << std::endl;
        return nullptr;
    }
    
    // Create a texture with the specified dimensions
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        std::cerr << "Failed to create placeholder texture: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    // Store the current render target
    SDL_Texture* previousTarget = SDL_GetRenderTarget(renderer);
    
    // Set the render target to our new texture
    if (SDL_SetRenderTarget(renderer, texture) != 0) {
        std::cerr << "Failed to set render target: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(texture);
        return nullptr;
    }
    
    // Fill with background color
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);
    
    // Draw a border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect border = {0, 0, width, height};
    SDL_RenderDrawRect(renderer, &border);
    
    // Draw the label text
    if (!label.empty()) {
        drawPixelText(renderer, label, width/2 - (label.length() * 3), height/2 - 3, {0, 0, 0, 255});
    }
    
    // Restore the previous render target
    if (SDL_SetRenderTarget(renderer, previousTarget) != 0) {
        std::cerr << "Failed to restore render target: " << SDL_GetError() << std::endl;
        SDL_DestroyTexture(texture);
        return nullptr;
    }
    
    return texture;
}

// Render the intro screen
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
}

// Render the plant view screen with weather and day/night cycle
void renderPlantViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Plant& plant,
                         const Player& player, PlantNavigationButtons& navButtons, 
                         WeatherType weather, DayNightType dayNight, 
                         const std::vector<Background>& backgrounds,
                         const std::vector<Raindrop>& raindrops) {
    if (!renderer) return;
    
    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
    SDL_RenderClear(renderer);
    
    // Draw background based on weather and time of day
    SDL_Color bgColor;
    if (dayNight == DayNightType::NIGHT) {
        bgColor = {25, 25, 112, 255}; // Midnight blue for night
    } else {
        switch (weather) {
            case WeatherType::SUNNY:
                bgColor = {135, 206, 235, 255}; // Sky blue
                break;
            case WeatherType::RAINY:
                bgColor = {105, 105, 105, 255}; // Dark gray
                break;
            case WeatherType::CLOUDY:
                bgColor = {176, 196, 222, 255}; // Light steel blue
                break;
            case WeatherType::WINDY:
                bgColor = {176, 224, 230, 255}; // Powder blue
                break;
        }
    }
    
    // Fill background with color
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - TOOLBAR_HEIGHT};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Draw background texture if available
    if (!backgrounds.empty() && backgrounds[0].texture) {
        // Scale to fill width completely
        double scale = static_cast<double>(SCREEN_WIDTH) / backgrounds[0].width;
        int scaledHeight = static_cast<int>(backgrounds[0].height * scale);
        
        // Center vertically if needed
        int y = std::max(0, (SCREEN_HEIGHT - TOOLBAR_HEIGHT - scaledHeight) / 2);
        
        renderTexture(renderer, backgrounds[0].texture, 0, y, nullptr, scale);
    }
    
    // Draw weather effects
    if (weather == WeatherType::RAINY) {
        SDL_SetRenderDrawColor(renderer, 173, 216, 230, 150);
        for (const auto& drop : raindrops) {
            SDL_RenderDrawLine(renderer, drop.x, drop.y, drop.x, drop.y + drop.length);
        }
    }
    
    // Calculate plant scale and position to ensure it's fully visible
    // Use a smaller percentage of screen space to ensure plants don't touch edges
    double maxWidth = SCREEN_WIDTH * 0.5;  // Reduced from 0.6
    double maxHeight = (SCREEN_HEIGHT - TOOLBAR_HEIGHT) * 0.5;  // Reduced from 0.6
    
    double scaleWidth = maxWidth / plant.width;
    double scaleHeight = maxHeight / plant.height;
    double plantScale = std::min(scaleWidth, scaleHeight);
    
    int scaledPlantWidth = static_cast<int>(plant.width * plantScale);
    int scaledPlantHeight = static_cast<int>(plant.height * plantScale);
    
    // Center the plant on screen with additional padding
    int plantX = (SCREEN_WIDTH - scaledPlantWidth) / 2;
    int plantY = ((SCREEN_HEIGHT - TOOLBAR_HEIGHT) - scaledPlantHeight) / 2;
    
    // Draw the plant
    if (plant.texture) {
        renderTexture(renderer, plant.texture, plantX, plantY, nullptr, plantScale);
    }
    
    // Draw plant name at the top
    std::string plantName = plant.name;
    int textX = centerTextX(plantName, SCREEN_WIDTH);
    drawPixelText(renderer, plantName, textX, 10, palette.white);
    
    // Draw token count in top right corner
    std::string tokenText = std::to_string(player.coins) + " coins";
    SDL_Rect tokenDims = getTextDimensions(tokenText);
    drawPixelText(renderer, tokenText, SCREEN_WIDTH - tokenDims.w - 10, 10, palette.yellow);
    
    // Position navigation buttons at the bottom
    int buttonY = SCREEN_HEIGHT - TOOLBAR_HEIGHT - 40;
    int buttonSpacing = 10;
    int buttonSize = 24;

    // Calculate total width of buttons
    int totalButtonsWidth = (buttonSize + buttonSpacing) * 4 - buttonSpacing; // 4 buttons
    int startX = (SCREEN_WIDTH - totalButtonsWidth) / 2;

    // Position buttons
    SDL_Rect prevRect = {startX, buttonY, buttonSize, buttonSize};
    SDL_Rect nextRect = {startX + buttonSize + buttonSpacing, buttonY, buttonSize, buttonSize};
    SDL_Rect mapRect = {startX + (buttonSize + buttonSpacing) * 2, buttonY, buttonSize, buttonSize};
    SDL_Rect storeRect = {startX + (buttonSize + buttonSpacing) * 3, buttonY, buttonSize, buttonSize};
    
    navButtons.prevPlantButton.rect = prevRect;
    navButtons.nextPlantButton.rect = nextRect;
    navButtons.mapButton.rect = mapRect;
    navButtons.storeButton.rect = storeRect;

    // Draw navigation buttons
    drawButton(renderer, navButtons.prevPlantButton, palette);
    drawButton(renderer, navButtons.nextPlantButton, palette);
    drawButton(renderer, navButtons.mapButton, palette);
    drawButton(renderer, navButtons.storeButton, palette);

    // Draw toolbar
    SDL_Rect toolbarRect = {0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT};
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, 
                          palette.darkest.b, palette.darkest.a);
    SDL_RenderFillRect(renderer, &toolbarRect);
    SDL_SetRenderDrawColor(renderer, palette.lightest.r, palette.lightest.g, 
                          palette.lightest.b, palette.lightest.a);
    SDL_RenderDrawRect(renderer, &toolbarRect);
}

// Render the menu view screen
void renderMenuViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                        const std::vector<Plant>& plants, const Player& player,
                        const std::vector<Button>& menuGridButtons, 
                        const Button& prevPageButton, const Button& nextPageButton,
                        int currentPage, int plantsPerPage) {
    if (!renderer) return;
    
    // Load and draw garden background
    SDL_Texture* gardenBg = loadTexture(renderer, "assets/bg_garden.png");
    if (gardenBg) {
        // Scale background to fit screen
        int bgWidth, bgHeight;
        SDL_QueryTexture(gardenBg, nullptr, nullptr, &bgWidth, &bgHeight);
        double scaleWidth = static_cast<double>(SCREEN_WIDTH) / bgWidth;
        double scaleHeight = static_cast<double>(SCREEN_HEIGHT) / bgHeight;
        double scale = std::max(scaleWidth, scaleHeight);
        
        int scaledWidth = static_cast<int>(bgWidth * scale);
        int scaledHeight = static_cast<int>(bgHeight * scale);
        
        // Center the background
        int x = (SCREEN_WIDTH - scaledWidth) / 2;
        int y = (SCREEN_HEIGHT - scaledHeight) / 2;
        
        renderTexture(renderer, gardenBg, x, y, nullptr, scale);
        SDL_DestroyTexture(gardenBg);
    } else {
        // Fallback solid color if background fails to load
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255); // Forest green
        SDL_RenderClear(renderer);
    }
    
    // Constants for plant layout
    const int PLANTS_PER_ROW = 3;  // Changed from 4 to 3
    const int SIDE_MARGIN = 30;    // Increased margin for better spacing
    const int TOP_MARGIN = 60;     // Increased top margin
    const int ROW_SPACING = 120;   // Increased row spacing for larger plants
    
    // Calculate plant size and spacing to fit 3 plants per row with margins
    const int AVAILABLE_WIDTH = SCREEN_WIDTH - (2 * SIDE_MARGIN);
    const int PLANT_SPACING = 20;  // Increased spacing between plants
    const int TOTAL_SPACING = PLANT_SPACING * (PLANTS_PER_ROW - 1);
    const int PLANT_BASE_SIZE = (AVAILABLE_WIDTH - TOTAL_SPACING) / PLANTS_PER_ROW;
    
    // Draw all plants in grid
    for (size_t i = 0; i < plants.size(); i++) {
        int row = i / PLANTS_PER_ROW;
        int col = i % PLANTS_PER_ROW;
        
        // Calculate position for each plant
        int x = SIDE_MARGIN + col * (PLANT_BASE_SIZE + PLANT_SPACING);
        int y = TOP_MARGIN + row * ROW_SPACING;
        
        // Update button position
        if (i < menuGridButtons.size()) {
            const_cast<Button&>(menuGridButtons[i]).rect = {
                x,
                y,
                PLANT_BASE_SIZE,
                PLANT_BASE_SIZE
            };
        }
        
        // Draw plant
        if (plants[i].texture && plants[i].width > 0 && plants[i].height > 0) {
            double scale = static_cast<double>(PLANT_BASE_SIZE) / std::max(plants[i].width, plants[i].height);
            scale *= 0.85;  // Slightly larger scale factor than before (was 0.8)
            
            int scaledWidth = static_cast<int>(plants[i].width * scale);
            int scaledHeight = static_cast<int>(plants[i].height * scale);
            
            // Center plant within its grid cell
            int plantX = x + (PLANT_BASE_SIZE - scaledWidth) / 2;
            int plantY = y + (PLANT_BASE_SIZE - scaledHeight) / 2;
            
            renderTexture(renderer, plants[i].texture, plantX, plantY, nullptr, scale);
        }
    }
}

// Update the drawPixelText function to use the new renderText
void drawPixelText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    renderText(renderer, text, x, y, color);
}

// Function to render a texture to the screen
void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, SDL_Rect* clip, double scale) {
    if (!renderer || !texture) return;  // Add null checks
    
    // Enable alpha blending for transparency
    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
        std::cerr << "Failed to set texture blend mode: " << SDL_GetError() << std::endl;
        return;
    }
    
    // Set rendering space
    SDL_Rect renderQuad = {x, y, 0, 0};
    
    // Set clip rendering dimensions
    if (clip != nullptr) {
        renderQuad.w = static_cast<int>(clip->w * scale);
        renderQuad.h = static_cast<int>(clip->h * scale);
    } else {
        // Query texture to get its width and height
        int w, h;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) != 0) {
            std::cerr << "Failed to query texture: " << SDL_GetError() << std::endl;
            return;
        }
        renderQuad.w = static_cast<int>(w * scale);
        renderQuad.h = static_cast<int>(h * scale);
    }
    
    // Render to screen
    if (SDL_RenderCopy(renderer, texture, clip, &renderQuad) != 0) {
        std::cerr << "Failed to render texture: " << SDL_GetError() << std::endl;
    }
}

// Function to load backgrounds
std::vector<Background> loadBackgrounds(SDL_Renderer* renderer) {
    std::vector<Background> backgrounds;
    
    // Load background textures
    const std::vector<std::string> bgFiles = {
        "assets/bg_day_sunny.png",
        // "assets/bg_day_rainy.png",
        // "assets/bg_day_cloudy.png",
        // "assets/bg_day_windy.png",
        // "assets/bg_night.png"
    };
    
    for (const auto& file : bgFiles) {
        Background bg;
        bg.filename = file;
        
        // Try to load the background texture
        bg.texture = loadTexture(renderer, file);
        if (bg.texture == nullptr) {
            std::cerr << "Failed to load background texture: " << file << std::endl;
            continue;
        }
        
        // Get texture dimensions
        if (SDL_QueryTexture(bg.texture, nullptr, nullptr, &bg.width, &bg.height) != 0) {
            std::cerr << "Failed to query texture dimensions for: " << file << std::endl;
        }
        
        // Set background name based on filename
        bg.name = file.substr(7, file.length() - 11); // Remove "assets/" and ".png"
        
        // Use move semantics when adding to vector
        backgrounds.push_back(std::move(bg));
    }
    
    return backgrounds;
}

// Add new function to render the map screen
void renderMapScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                    const std::vector<Button>& locationButtons) {
    if (!renderer) return;
    
    // Clear screen
    SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
    SDL_RenderClear(renderer);
    
    // Draw map background
    SDL_Texture* mapTexture = loadTexture(renderer, "assets/map.png");
    if (mapTexture) {
        // Scale map to fit screen width while maintaining aspect ratio
        int mapWidth, mapHeight;
        SDL_QueryTexture(mapTexture, nullptr, nullptr, &mapWidth, &mapHeight);
        double scale = static_cast<double>(SCREEN_WIDTH) / mapWidth;
        int scaledHeight = static_cast<int>(mapHeight * scale);
        
        // Center vertically
        int y = (SCREEN_HEIGHT - scaledHeight) / 2;
        
        renderTexture(renderer, mapTexture, 0, y, nullptr, scale);
        SDL_DestroyTexture(mapTexture);
    }
    
    // Draw location buttons with updated positions
    for (const auto& button : locationButtons) {
        // Draw button background
        SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
        SDL_RenderFillRect(renderer, &button.rect);
        
        // Draw button border
        SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
        SDL_RenderDrawRect(renderer, &button.rect);
        
        // Draw location icon
        std::string icon;
        if (button.rect.x < SCREEN_WIDTH/2) {
            // Left side
            if (button.rect.y < SCREEN_HEIGHT/2) {
                icon = "🏠"; // House top left
            } else {
                icon = "🌾"; // Pasture bottom left
            }
        } else {
            // Right side
            if (button.rect.y < SCREEN_HEIGHT/2) {
                icon = "🌿"; // Greenhouse top right
            } else {
                icon = "🏪"; // Store bottom right
            }
        }
        
        // Center the icon in the button
        int iconX = button.rect.x + (button.rect.w - 16) / 2;
        int iconY = button.rect.y + (button.rect.h - 16) / 2;
        drawPixelText(renderer, icon, iconX, iconY, palette.white);
    }
    
    // Draw toolbar background
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_Rect toolbarRect = {0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT};
    SDL_RenderFillRect(renderer, &toolbarRect);
    
    // Draw back button
    Button backButton = {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
    SDL_RenderFillRect(renderer, &backButton.rect);
    drawPixelText(renderer, "←", 
                 backButton.rect.x + 8,
                 backButton.rect.y + 8,
                 palette.white);
}

// Function to render a location screen
void renderLocationScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                         const std::string& locationName, const SDL_Color& bgColor,
                         const Button& backButton) {
    if (!renderer) return;
    
    // Clear screen with location background color
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(renderer);
    
    // Draw location name at the top
    int textX = centerTextX(locationName, SCREEN_WIDTH);
    drawPixelText(renderer, locationName, textX, 10, palette.white);
    
    // Draw toolbar background
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, palette.darkest.b, palette.darkest.a);
    SDL_Rect toolbarRect = {0, SCREEN_HEIGHT - TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT};
    SDL_RenderFillRect(renderer, &toolbarRect);
    
    // Draw back button
    SDL_SetRenderDrawColor(renderer, palette.medium.r, palette.medium.g, palette.medium.b, palette.medium.a);
    SDL_RenderFillRect(renderer, &backButton.rect);
    drawPixelText(renderer, "←", 
                 backButton.rect.x + 8,
                 backButton.rect.y + 8,
                 palette.white);
}

// Simpler text wrapping function that doesn't require stringstream
std::vector<std::string> wrapText(const std::string& text, int maxWidth) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::string currentWord;
    
    for (char c : text) {
        if (c == ' ') {
            SDL_Rect testSize = getTextDimensions(currentLine + (currentLine.empty() ? "" : " ") + currentWord);
            if (testSize.w > maxWidth && !currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine = currentWord;
            } else {
                if (!currentLine.empty()) currentLine += " ";
                currentLine += currentWord;
            }
            currentWord = "";
        } else {
            currentWord += c;
        }
    }
    
    // Handle the last word
    if (!currentWord.empty()) {
        SDL_Rect testSize = getTextDimensions(currentLine + (currentLine.empty() ? "" : " ") + currentWord);
        if (testSize.w > maxWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            lines.push_back(currentWord);
        } else {
            if (!currentLine.empty()) currentLine += " ";
            currentLine += currentWord;
            lines.push_back(currentLine);
        }
    } else if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    
    return lines;
}

void renderStoreScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                      const std::vector<Plant>& plants, const Player& player,
                      const Button& backButton, const std::string& shopkeeperText,
                      const Button& yesButton, const Button& noButton,
                      int selectedPlantIndex, int offerAmount) {
    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, 
                          palette.background.b, palette.background.a);
    SDL_RenderClear(renderer);

    // Draw store background
    SDL_Texture* storeTexture = loadTexture(renderer, "assets/store.png");
    if (storeTexture) {
        // Scale store background to fit screen width while maintaining aspect ratio
        int storeWidth, storeHeight;
        SDL_QueryTexture(storeTexture, nullptr, nullptr, &storeWidth, &storeHeight);
        double scale = static_cast<double>(SCREEN_WIDTH) / storeWidth;
        int scaledHeight = static_cast<int>(storeHeight * scale);
        
        // Center vertically
        int y = (SCREEN_HEIGHT - scaledHeight) / 2;
        
        renderTexture(renderer, storeTexture, 0, y, nullptr, scale);
        SDL_DestroyTexture(storeTexture);
    }

    // Draw back button
    drawButton(renderer, backButton, palette);

    // Draw dialog box with white background
    int dialogBoxY = SCREEN_HEIGHT - TOOLBAR_HEIGHT - 120;
    SDL_Rect dialogBox = {10, dialogBoxY, SCREEN_WIDTH - 20, 100};
    
    // Draw white background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &dialogBox);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, palette.darkest.r, palette.darkest.g, 
                          palette.darkest.b, palette.darkest.a);
    SDL_RenderDrawRect(renderer, &dialogBox);

    // Calculate text wrapping width
    const int TEXT_MARGIN = 10;
    const int WRAP_WIDTH = dialogBox.w - (TEXT_MARGIN * 2);
    
    // Draw wrapped shopkeeper text in black
    SDL_Color textColor = {0, 0, 0, 255};
    std::vector<std::string> wrappedText = wrapText(shopkeeperText, WRAP_WIDTH);
    int lineY = dialogBoxY + TEXT_MARGIN;
    for (const auto& line : wrappedText) {
        drawPixelText(renderer, line, dialogBox.x + TEXT_MARGIN, lineY, textColor);
        lineY += 20;  // Line spacing
    }

    // Draw selected plant info and visual if showing offer
    if (selectedPlantIndex >= 0 && selectedPlantIndex < plants.size()) {
        // Draw plant info
        std::string plantInfo = "Plant: " + plants[selectedPlantIndex].name;
        drawPixelText(renderer, plantInfo, dialogBox.x + TEXT_MARGIN, lineY, textColor);
        
        if (offerAmount > 0) {
            std::string offerText = "Offer: " + std::to_string(offerAmount) + " coins";
            drawPixelText(renderer, offerText, dialogBox.x + TEXT_MARGIN, lineY + 20, textColor);
        }
        
        // Draw the selected plant texture
        if (plants[selectedPlantIndex].texture) {
            const int PLANT_DISPLAY_SIZE = 48;  // Size for plant preview
            int plantX = SCREEN_WIDTH - PLANT_DISPLAY_SIZE - 20;  // Position on right side
            int plantY = dialogBoxY + (dialogBox.h - PLANT_DISPLAY_SIZE) / 2;  // Centered vertically in dialog
            
            // Calculate scale to fit in display size
            double scaleW = static_cast<double>(PLANT_DISPLAY_SIZE) / plants[selectedPlantIndex].width;
            double scaleH = static_cast<double>(PLANT_DISPLAY_SIZE) / plants[selectedPlantIndex].height;
            double scale = std::min(scaleW, scaleH);
            
            renderTexture(renderer, plants[selectedPlantIndex].texture, 
                         plantX, plantY, nullptr, scale);
        }
    }

    // Position Yes/No buttons below dialog box
    const int BUTTON_WIDTH = 40;
    const int BUTTON_HEIGHT = 20;
    const int BUTTON_SPACING = 20;
    
    // Center the buttons horizontally
    int totalWidth = (BUTTON_WIDTH * 2) + BUTTON_SPACING;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    // Position buttons below the dialog box
    int buttonY = dialogBoxY + dialogBox.h + 10;
    
    // Update button positions
    const_cast<Button&>(yesButton).rect = {
        startX,
        buttonY,
        BUTTON_WIDTH,
        BUTTON_HEIGHT
    };
    
    const_cast<Button&>(noButton).rect = {
        startX + BUTTON_WIDTH + BUTTON_SPACING,
        buttonY,
        BUTTON_WIDTH,
        BUTTON_HEIGHT
    };

    // Draw Yes/No buttons with just text
    drawPixelText(renderer, "[Y]", startX + 5, buttonY + 5, palette.white);
    drawPixelText(renderer, "[N]", startX + BUTTON_WIDTH + BUTTON_SPACING + 5, buttonY + 5, palette.white);
} 