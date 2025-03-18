#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cmath>
#include "../include/game.h"
#include "../include/render.h"
#include <ctime>
#include <random>
#include <algorithm>

// Forward declarations for rendering functions
void drawPixelText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
SDL_Texture* createPlaceholderBackground(SDL_Renderer* renderer, const SDL_Color& bgColor, const std::string& label, int width, int height);
std::vector<Background> loadBackgrounds(SDL_Renderer* renderer);
void renderIntroScreen(SDL_Renderer* renderer, const ColorPalette& palette);
void renderPlantViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const Plant& plant, const Player& player, 
                         const PlantNavigationButtons& navButtons, WeatherType weather, DayNightType dayNight,
                         const std::vector<Background>& backgrounds, const std::vector<Raindrop>& raindrops);
void renderMenuViewScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::vector<Plant>& plants, 
                         const Player& player, const std::vector<Button>& menuGridButtons, 
                         const Button& prevPageButton, const Button& nextPageButton, int currentPage, int plantsPerPage);
void renderMapScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::vector<Button>& locationButtons);
void renderLocationScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::string& locationName, 
                         const SDL_Color& bgColor, const Button& backButton);
void renderStoreScreen(SDL_Renderer* renderer, const ColorPalette& palette, const std::vector<Plant>& plants, const Player& player,
                     const Button& backButton, const std::string& shopkeeperText,
                     const Button& yesButton, const Button& noButton,
                     int selectedPlantIndex, int offerAmount);

// Function to load plants
std::vector<Plant> loadPlants(SDL_Renderer* renderer) {
    std::vector<Plant> plants;
    const int TOTAL_PLANTS = 8; // Reduced number of plants
    
    // Create a default texture for plants that fail to load
    SDL_Texture* defaultTexture = createPlaceholderBackground(renderer, {100, 100, 100, 255}, "?", 32, 32);
    if (!defaultTexture) {
        std::cerr << "Failed to create default texture!" << std::endl;
        return plants;
    }
    
    bool defaultTextureUsed = false;
    
    for (int i = 1; i <= TOTAL_PLANTS; i++) {
        Plant plant;
        plant.name = "Plant " + std::to_string(i);
        plant.filename = "assets/plant_" + std::to_string(i) + ".png";
        plant.isOwned = true; // Make all plants owned by default
        
        // Try to load the plant texture
        plant.texture = loadTexture(renderer, plant.filename);
        if (plant.texture == nullptr) {
            std::cerr << "Failed to load plant texture: " << plant.filename << ", using default" << std::endl;
            plant.texture = defaultTexture;
            defaultTextureUsed = true;
        }
        
        // Set default dimensions
        plant.width = 32;
        plant.height = 32;
        
        // Try to get actual dimensions if texture loaded successfully
        if (plant.texture != defaultTexture) {
            if (SDL_QueryTexture(plant.texture, nullptr, nullptr, &plant.width, &plant.height) != 0) {
                std::cerr << "Failed to query texture dimensions for: " << plant.filename << std::endl;
            }
        }
        
        // Assign a random preferred weather to each plant
        plant.preferredWeather = static_cast<WeatherType>(rand() % 4);
        
        // Use move semantics when adding to vector
        plants.push_back(std::move(plant));
    }
    
    // If default texture was used, we need to create a new one for each plant that used it
    if (defaultTextureUsed) {
        for (auto& plant : plants) {
            if (plant.texture == defaultTexture) {
                plant.texture = createPlaceholderBackground(renderer, {100, 100, 100, 255}, "?", 32, 32);
            }
        }
        SDL_DestroyTexture(defaultTexture);
    }
    
    return plants;
}

// Define buttons as global variables
PlantNavigationButtons navButtons;
Button continueButton;
Button prevPageButton;
Button nextPageButton;
Button okButton;
std::vector<Button> inventoryGridButtons;  // Renamed from menuGridButtons

// Add global variables for celebration
std::vector<Particle> celebrationParticles;
Uint32 celebrationStartTime = 0;

// Main function
int main(int argc, char* args[]) {
    // Initialize random seed
    srand(time(NULL));
    
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
    
    // Initialize font system
    if (!initFont()) {
        std::cerr << "Failed to initialize font system!" << std::endl;
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
    
    // Create Player
    Player player;
    
    // Load backgrounds
    std::vector<Background> backgrounds = loadBackgrounds(renderer);
    WeatherType currentWeather = WeatherType::SUNNY;
    DayNightType currentDayNight = DayNightType::DAY;
    Uint32 lastWeatherChange = SDL_GetTicks();
    
    // Load plants
    std::vector<Plant> plants = loadPlants(renderer);
    
    // Create game state data
    GameStateData state;
    state.currentState = GameState::INTRO;
    // Move plants into state instead of assignment
    state.plants = std::move(plants);
    
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
                if (index < state.plants.size()) {
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
    
    // Initialize navigation buttons
    navButtons.prevPlantButton = {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, 
                                  MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
    navButtons.nextPlantButton = {{5 + MENU_BUTTON_SIZE, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, 
                                  MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
    navButtons.mapButton = {{SCREEN_WIDTH - MENU_BUTTON_SIZE - 5, 
                           SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, 
                           MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
    
    // Initialize location buttons for map screen
    std::vector<Button> locationButtons;
    const int LOCATION_BUTTON_SIZE = 40;
    const int PADDING = 10;
    
    // House (top left)
    locationButtons.push_back({{PADDING, PADDING, LOCATION_BUTTON_SIZE, LOCATION_BUTTON_SIZE}, false});
    // Greenhouse (top right)
    locationButtons.push_back({{SCREEN_WIDTH - LOCATION_BUTTON_SIZE - PADDING, PADDING, 
                               LOCATION_BUTTON_SIZE, LOCATION_BUTTON_SIZE}, false});
    // Pasture (bottom left)
    locationButtons.push_back({{PADDING, SCREEN_HEIGHT - LOCATION_BUTTON_SIZE - PADDING, 
                               LOCATION_BUTTON_SIZE, LOCATION_BUTTON_SIZE}, false});
    // Store (bottom right)
    locationButtons.push_back({{SCREEN_WIDTH - LOCATION_BUTTON_SIZE - PADDING, 
                               SCREEN_HEIGHT - LOCATION_BUTTON_SIZE - PADDING, 
                               LOCATION_BUTTON_SIZE, LOCATION_BUTTON_SIZE}, false});
    
    // Setup raindrops for animation
    const int MAX_RAINDROPS = 100;
    std::vector<Raindrop> raindrops(MAX_RAINDROPS);
    
    // Initialize raindrops
    for (int i = 0; i < MAX_RAINDROPS; i++) {
        raindrops[i].x = rand() % SCREEN_WIDTH;
        raindrops[i].y = rand() % SCREEN_HEIGHT;
        raindrops[i].speed = 2.0f + (rand() % 20) / 10.0f;
        raindrops[i].length = 5 + rand() % 10;
    }
    
    // Main loop flag
    bool quit = false;
    
    // Event handler
    SDL_Event e;
    
    // Color palette
    ColorPalette palette;
    
    // Main loop
    while (!quit) {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                
                if (state.currentState == GameState::INTRO) {
                    player.selectedPlantIndex = 0;
                    state.currentState = GameState::PLANT_VIEW;
                }
                else if (state.currentState == GameState::PLANT_VIEW) {
                    // Handle plant navigation
                    if (navButtons.prevPlantButton.contains(mouseX, mouseY)) {
                        player.selectedPlantIndex = (player.selectedPlantIndex - 1 + state.plants.size()) % state.plants.size();
                    }
                    else if (navButtons.nextPlantButton.contains(mouseX, mouseY)) {
                        player.selectedPlantIndex = (player.selectedPlantIndex + 1) % state.plants.size();
                    }
                    else if (navButtons.mapButton.contains(mouseX, mouseY)) {
                        state.currentState = GameState::MAP_VIEW;
                    } else if (navButtons.storeButton.contains(mouseX, mouseY)) {
                        state.currentState = GameState::STORE_VIEW;
                        resetStoreState(state);
                    }
                }
                else if (state.currentState == GameState::INVENTORY_VIEW) {
                    // Check if a plant was clicked in inventory
                    for (size_t i = 0; i < inventoryGridButtons.size(); i++) {
                        if (inventoryGridButtons[i].contains(mouseX, mouseY)) {
                            player.selectedPlantIndex = currentPage * plantsPerPage + i;
                            state.currentState = GameState::PLANT_VIEW;
                            break;
                        }
                    }
                    // Handle pagination
                    if (prevPageButton.contains(mouseX, mouseY)) {
                        currentPage = std::max(0, currentPage - 1);
                        updateMenuGrid();
                    }
                    else if (nextPageButton.contains(mouseX, mouseY)) {
                        currentPage = std::min(static_cast<int>((state.plants.size() - 1) / plantsPerPage), currentPage + 1);
                        updateMenuGrid();
                    }
                }
                else if (state.currentState == GameState::MAP_VIEW) {
                    // Handle location button clicks
                    for (size_t i = 0; i < locationButtons.size(); i++) {
                        if (locationButtons[i].contains(mouseX, mouseY)) {
                            switch (i) {
                                case 0: // House
                                    state.currentState = GameState::PLANT_VIEW; // Home goes to plant view
                                    break;
                                case 1: // Greenhouse
                                    state.currentState = GameState::INVENTORY_VIEW; // Greenhouse shows all plants
                                    break;
                                case 2: // Pasture
                                    state.currentState = GameState::PASTURE_VIEW;
                                    break;
                                case 3: // Store
                                    state.currentState = GameState::STORE_VIEW;
                                    break;
                            }
                            break;
                        }
                    }
                    // Handle back button
                    Button backButton = {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
                    if (backButton.contains(mouseX, mouseY)) {
                        state.currentState = GameState::PLANT_VIEW;
                    }
                }
                else if (state.currentState == GameState::HOUSE_VIEW || state.currentState == GameState::GREENHOUSE_VIEW || 
                         state.currentState == GameState::PASTURE_VIEW || state.currentState == GameState::STORE_VIEW) {
                    Button backButton = {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
                    if (backButton.contains(mouseX, mouseY)) {
                        state.currentState = GameState::MAP_VIEW;
                    }
                    
                    // Handle store interactions
                    if (state.currentState == GameState::STORE_VIEW) {
                        handleStoreInteraction(state, mouseX, mouseY);
                    }
                }
            }
        }
        
        // Get current time for animations and timers
        Uint32 currentTime = SDL_GetTicks();
        
        // Check if it's time to change weather
        if (currentTime - lastWeatherChange >= WEATHER_CHANGE_INTERVAL) {
            currentWeather = static_cast<WeatherType>(rand() % 4);
            lastWeatherChange = currentTime;
        }
        
        // Update day/night based on system time
        time_t now = time(0);
        struct tm *ltm = localtime(&now);
        int hour = ltm->tm_hour;
        
        // Set day/night based on hour (e.g., 6 AM to 6 PM is day)
        currentDayNight = (hour >= 6 && hour < 18) ? DayNightType::DAY : DayNightType::NIGHT;
        
        // Update raindrops if weather is rainy
        if (currentWeather == WeatherType::RAINY) {
            for (auto& drop : raindrops) {
                drop.y += drop.speed;
                if (drop.y > SCREEN_HEIGHT) {
                    drop.y = -drop.length;
                    drop.x = rand() % SCREEN_WIDTH;
                }
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
        SDL_RenderClear(renderer);
        
        // Render the current state
        switch (state.currentState) {
            case GameState::INTRO:
                renderIntroScreen(renderer, palette);
                break;
                
            case GameState::PLANT_VIEW:
                if (player.selectedPlantIndex >= 0 && player.selectedPlantIndex < state.plants.size()) {
                    renderPlantViewScreen(renderer, palette, state.plants[player.selectedPlantIndex], player,
                                         navButtons, currentWeather, currentDayNight,
                                         backgrounds, raindrops);
                } else {
                    // If no plant is selected, go back to inventory view
                    state.currentState = GameState::INVENTORY_VIEW;
                }
                break;
                
            case GameState::INVENTORY_VIEW:
                renderMenuViewScreen(renderer, palette, state.plants, player, inventoryGridButtons,
                                   prevPageButton, nextPageButton, currentPage, plantsPerPage);
                break;
                
            case GameState::MAP_VIEW:
                renderMapScreen(renderer, palette, locationButtons);
                break;
                
            case GameState::HOUSE_VIEW:
                renderLocationScreen(renderer, palette, "House", {139, 69, 19, 255}, 
                                   {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false});
                break;
                
            case GameState::GREENHOUSE_VIEW:
                renderLocationScreen(renderer, palette, "Greenhouse", {34, 139, 34, 255},
                                   {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false});
                break;
                
            case GameState::PASTURE_VIEW:
                renderLocationScreen(renderer, palette, "Pasture", {144, 238, 144, 255},
                                   {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false});
                break;
                
            case GameState::STORE_VIEW:
                renderStoreScreen(renderer, palette, state.plants, player,
                                 state.storeState.backButton,
                                 state.storeState.shopkeeperText,
                                 state.storeState.yesButton,
                                 state.storeState.noButton,
                                 state.storeState.selectedPlantIndex,
                                 state.storeState.offerAmount);
                break;
                
            default:
                renderIntroScreen(renderer, palette);
                break;
        }
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Cap frame rate
        SDL_Delay(16);
    }
    
    // Cleanup and exit
    SDL_StopTextInput();
    cleanupFont();
    
    // Clean up plant textures
    for (auto& plant : state.plants) {
        if (plant.texture) {
            SDL_DestroyTexture(plant.texture);
            plant.texture = nullptr;
        }
    }
    
    // Clean up background textures
    for (auto& bg : backgrounds) {
        if (bg.texture) {
            SDL_DestroyTexture(bg.texture);
            bg.texture = nullptr;
        }
    }
    
    // Clean up SDL resources
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
    SDL_DestroyWindow(window);
    window = nullptr;
    IMG_Quit();
    SDL_Quit();
    
    return 0;
}

void generateOffer(GameStateData& state) {
    // Random number generator for offer amount
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(50, 200);
    state.storeState.offerAmount = dis(gen);

    // Get the selected plant's name
    std::string plantName = state.plants[state.storeState.selectedPlantIndex].name;

    // Generate text based on offer amount
    if (state.storeState.offerAmount > 150) {
        state.storeState.shopkeeperText = "Wow, that " + plantName + " looks amazing! I'll give you a great price!";
    } else if (state.storeState.offerAmount > 100) {
        state.storeState.shopkeeperText = "Hmm, that " + plantName + " is in good shape. I can offer a fair price.";
    } else {
        state.storeState.shopkeeperText = "Well, that " + plantName + " has seen better days... here's what I can offer.";
    }
}

void handleStoreInteraction(GameStateData& state, int x, int y) {
    if (state.storeState.backButton.contains(x, y)) {
        state.currentState = GameState::MAP_VIEW;
        resetStoreState(state);
        return;
    }

    if (!state.storeState.isAskingToSell && !state.storeState.isShowingOffer) {
        // Initial state - ask if they want to sell
        if (state.storeState.yesButton.contains(x, y)) {
            state.storeState.isAskingToSell = true;
            // Select a random owned plant
            std::vector<int> ownedIndices;
            for (size_t i = 0; i < state.plants.size(); i++) {
                if (state.plants[i].isOwned) {
                    ownedIndices.push_back(i);
                }
            }
            if (!ownedIndices.empty()) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, ownedIndices.size() - 1);
                state.storeState.selectedPlantIndex = ownedIndices[dis(gen)];
                generateOffer(state);
                state.storeState.isShowingOffer = true;
            } else {
                state.storeState.shopkeeperText = "You don't have any plants to sell!";
                state.storeState.isAskingToSell = false;
            }
        } else if (state.storeState.noButton.contains(x, y)) {
            state.currentState = GameState::MAP_VIEW;
            resetStoreState(state);
        }
    } else if (state.storeState.isShowingOffer) {
        // Showing offer state
        if (state.storeState.yesButton.contains(x, y)) {
            // Accept offer
            if (state.storeState.selectedPlantIndex >= 0 && 
                state.storeState.selectedPlantIndex < state.plants.size()) {
                std::string soldPlantName = state.plants[state.storeState.selectedPlantIndex].name;
                
                // Add coins to player's balance
                state.player.coins += state.storeState.offerAmount;
                
                // Clean up the plant's texture before removing it
                if (state.plants[state.storeState.selectedPlantIndex].texture) {
                    SDL_DestroyTexture(state.plants[state.storeState.selectedPlantIndex].texture);
                }
                
                // Remove the plant from the vector
                state.plants.erase(state.plants.begin() + state.storeState.selectedPlantIndex);
                
                // Update selected plant index if needed
                if (state.player.selectedPlantIndex >= state.plants.size()) {
                    state.player.selectedPlantIndex = std::max(0, static_cast<int>(state.plants.size()) - 1);
                } else if (state.player.selectedPlantIndex > state.storeState.selectedPlantIndex) {
                    state.player.selectedPlantIndex--; // Adjust for removed plant
                }
                
                state.storeState.shopkeeperText = "Great! " + soldPlantName + " will have a good home. Come back soon!";
                state.storeState.isShowingOffer = false;
                // Reset after a delay
                SDL_Delay(2000);
                state.currentState = GameState::MAP_VIEW;
                resetStoreState(state);
            }
        } else if (state.storeState.noButton.contains(x, y)) {
            // Reject offer
            std::string rejectedPlantName = state.plants[state.storeState.selectedPlantIndex].name;
            state.storeState.shopkeeperText = "No deal on the " + rejectedPlantName + "? Maybe next time!";
            state.storeState.isShowingOffer = false;
            // Reset after a delay
            SDL_Delay(2000);
            resetStoreState(state);
        }
    }
}

void resetStoreState(GameStateData& state) {
    state.storeState.isAskingToSell = false;
    state.storeState.isShowingOffer = false;
    state.storeState.selectedPlantIndex = -1;
    state.storeState.offerAmount = 0;
    state.storeState.shopkeeperText = "Welcome! I'm interested in buying plants. Want to sell?";
    
    // Initialize back button
    state.storeState.backButton = {{5, SCREEN_HEIGHT - TOOLBAR_HEIGHT + 5, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}, false};
    
    // Initialize Yes/No buttons with default positions (will be updated in render)
    state.storeState.yesButton = {{0, 0, 40, 20}, false};
    state.storeState.noButton = {{0, 0, 40, 20}, false};
} 