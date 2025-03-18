#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <vector>
#include "game.h"

// Font initialization and cleanup
bool initFont();
void cleanupFont();

// Text rendering helper functions
SDL_Rect getTextDimensions(const std::string& text);
int centerTextX(const std::string& text, int containerWidth);
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);

// Function declarations for rendering
SDL_Texture* createPlaceholderBackground(SDL_Renderer* renderer, const SDL_Color& bgColor, const std::string& label, int width, int height);
std::vector<Background> loadBackgrounds(SDL_Renderer* renderer);

void renderIntroScreen(SDL_Renderer* renderer, const ColorPalette& palette);

void renderPlantViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Plant& plant,
                         const Player& player, PlantNavigationButtons& navButtons, 
                         WeatherType weather, DayNightType dayNight, 
                         const std::vector<Background>& backgrounds,
                         const std::vector<Raindrop>& raindrops);

void renderMenuViewScreen(SDL_Renderer* renderer, const ColorPalette& palette,
                        const std::vector<Plant>& plants, const Player& player,
                        const std::vector<Button>& menuGridButtons,
                        const Button& prevPageButton, const Button& nextPageButton,
                        int currentPage, int plantsPerPage);

void drawPixelText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);

// Helper functions
void renderTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, SDL_Rect* clip, double scale);
void drawButton(SDL_Renderer* renderer, const Button& button, const ColorPalette& palette);
void drawBgButton(SDL_Renderer* renderer, const Button& button, const ColorPalette& palette);
void drawPlantSelectionButton(SDL_Renderer* renderer, const Button& button, SDL_Texture* plantTexture, int plantWidth, int plantHeight, bool isSelected, const ColorPalette& palette);
void drawFertilizerIcon(SDL_Renderer* renderer, int x, int y, int size, const ColorPalette& palette);

void renderStoreScreen(SDL_Renderer* renderer, const ColorPalette& palette, 
                      const std::vector<Plant>& plants, const Player& player,
                      const Button& backButton, const std::string& shopkeeperText,
                      const Button& yesButton, const Button& noButton,
                      int selectedPlantIndex, int offerAmount);

#endif // RENDER_H 