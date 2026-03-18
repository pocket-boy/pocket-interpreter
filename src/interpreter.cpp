#include "interpreter.hpp"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>

constexpr int TILE_SIZE = 8;
constexpr int SCALE = 3;
constexpr int MAP_W = 32;
constexpr int MAP_H = 32;

struct Interpreter::Impl
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* tilemap = nullptr;
    SDL_Texture* spriteSheet = nullptr;
    SDL_Gamepad* gamepad = nullptr;

    std::vector<int> background;
    std::vector<int> foreground;
    std::vector<Sprite> sprites;
    
    int activeSprite = 0;
    Uint32 lastMove = 0;
    Uint32 delay = 200;
    Uint32 interval = 100;
    
    bool running = false;
    bool holding = false;
};

Interpreter::Interpreter()
{
    impl = new Impl();

    impl->background.resize(MAP_W * MAP_H, 0);
    impl->foreground.resize(MAP_W * MAP_H, -1);
}

Interpreter::~Interpreter()
{
    cleanup();
    delete impl;
}

bool Interpreter::initialize(int width, int height, const std::string& title)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    int count = 0;
    SDL_JoystickID* joysticks = SDL_GetJoysticks(&count);

    for (int i = 0; i < count; i++)
    {
        if (SDL_IsGamepad(joysticks[i]))
        {
            impl->gamepad = SDL_OpenGamepad(joysticks[i]);
            if (impl->gamepad)
            {
                std::cout << "Gamepad connected\n";
                break;
            }
            else
            {
                std::cerr << "Gamepad Error: " << SDL_GetError() << "\n";
            }
        }
    }

    SDL_free(joysticks);

    impl->window = SDL_CreateWindow(
        title.c_str(),
        width,
        height,
        0
    );

    if (!impl->window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    impl->renderer = SDL_CreateRenderer(impl->window, nullptr);

    if (!impl->renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderScale(impl->renderer, SCALE, SCALE);

    impl->running = true;

    return true;
}

bool Interpreter::loadTilemap(const std::string& fullPath)
{        
    impl->tilemap = IMG_LoadTexture(impl->renderer, fullPath.c_str());

    if (!impl->tilemap)
    {
        std::cerr << "Tilemap load Error: " << SDL_GetError() << " " << fullPath << std::endl;
        return false;
    }

    SDL_SetTextureBlendMode(impl->tilemap, SDL_BLENDMODE_BLEND);

    return true;
}

bool Interpreter::loadSpriteSheet(const std::string& fullPath)
{
    impl->spriteSheet = IMG_LoadTexture(impl->renderer, fullPath.c_str());

    if (!impl->spriteSheet)
    {
        std::cerr << "Sprite sheet load error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetTextureBlendMode(impl->spriteSheet, SDL_BLENDMODE_BLEND);

    return true;
}

void Interpreter::setBackgroundTile(int x, int y, int tile)
{
    impl->background[y * MAP_W + x] = tile;
}

void Interpreter::setForegroundTile(int x, int y, int tile)
{
    impl->foreground[y * MAP_W + x] = tile;
}

void Interpreter::addSprite(const Sprite& sprite)
{
    impl->sprites.push_back(sprite);
}

void Interpreter::clearSprites()
{
    impl->sprites.clear();
}

void Interpreter::tick()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            impl->running = false;

        if(event.type == SDL_EVENT_KEY_DOWN)
        {
            if (impl->sprites.empty()) continue;

            auto& s = impl->sprites[impl->activeSprite];
            int moveAmount = TILE_SIZE;

            SDL_Keycode key = event.key.key;

            switch (key)
            {
                case SDLK_W:
                    s.y -= moveAmount;
                    break;
                case SDLK_S:
                    s.y += moveAmount;
                    break;
                case SDLK_A:
                    s.x -= moveAmount;
                    break;
                case SDLK_D:
                    s.x += moveAmount;
                    break;
                case SDLK_Q:
                    impl->activeSprite = (impl->activeSprite + 1) % impl->sprites.size();
                    break;
            }

            s.x = std::max(0, std::min(s.x, MAP_W * TILE_SIZE - TILE_SIZE));
            s.y = std::max(0, std::min(s.y, MAP_H * TILE_SIZE - TILE_SIZE));
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
        {
            if (impl->sprites.empty()) continue;
            if (event.gbutton.button == SDL_GAMEPAD_BUTTON_NORTH)
            {
             impl->activeSprite =
                (impl->activeSprite + 1) % impl->sprites.size();
            }
        }
    }

    if (impl->gamepad && !impl->sprites.empty())
    {
        auto& s = impl->sprites[impl->activeSprite];

        int dx = 0;
        int dy = 0;

        Sint16 xAxis = SDL_GetGamepadAxis(impl->gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        Sint16 yAxis = SDL_GetGamepadAxis(impl->gamepad, SDL_GAMEPAD_AXIS_LEFTY);

        const int DEADZONE = 8000;

        if (xAxis > DEADZONE)  dx = 1;
        if (xAxis < -DEADZONE) dx = -1;
        if (yAxis > DEADZONE)  dy = 1;
        if (yAxis < -DEADZONE) dy = -1;

        if (SDL_GetGamepadButton(impl->gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))
        {
            dx = 0;
            dy = -1;
        }
        else if (SDL_GetGamepadButton(impl->gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
        {
            dx = 0;
            dy = 1;
        }
        else if (SDL_GetGamepadButton(impl->gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
        {
            dx = -1;
            dy = 0;
        }
        else if (SDL_GetGamepadButton(impl->gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
        {
            dx = 1;
            dy = 0;
        }

        Uint32 now = SDL_GetTicks();

        bool move = (dx != 0 || dy != 0);
        if (move)
        {
            if (!impl->holding)
            {
                s.x += dx * TILE_SIZE;
                s.y += dy * TILE_SIZE;

                impl->holding = true;
                impl->lastMove = now;
            }
            else
            {
                Uint32 elapsed = now - impl->lastMove;

                if (elapsed > impl->delay)
                {
                    s.x += dx * TILE_SIZE;
                    s.y += dy * TILE_SIZE;

                    impl->lastMove = now - (impl->delay - impl->interval);
                }
            }
        }
        else
        {
            impl->holding = false;
        }

        s.x = std::max(0, std::min(s.x, MAP_W * TILE_SIZE - TILE_SIZE));
        s.y = std::max(0, std::min(s.y, MAP_H * TILE_SIZE - TILE_SIZE));
    }
}

static void drawTile(SDL_Renderer* renderer, SDL_Texture* tex, int tileIndex, int x, int y)
{
    float w = 0;
    float h = 0;

    SDL_GetTextureSize(tex, &w, &h);

    int tilesPerRow = static_cast<int>(w) / TILE_SIZE;

    SDL_FRect src =
    {
        float((tileIndex % tilesPerRow) * TILE_SIZE),
        float((tileIndex / tilesPerRow) * TILE_SIZE),
        TILE_SIZE,
        TILE_SIZE
    };

    SDL_FRect dst =
    {
        float(x),
        float(y),
        float(TILE_SIZE),
        float(TILE_SIZE)
    };

    SDL_RenderTexture(renderer, tex, &src, &dst);
}

void Interpreter::render()
{
    SDL_RenderClear(impl->renderer);

    // Background tiles
    for (int y = 0; y < MAP_H; y++)
    for (int x = 0; x < MAP_W; x++)
    {
        int tile = impl->background[y * MAP_W + x];

        drawTile(
            impl->renderer,
            impl->tilemap,
            tile,
            x * TILE_SIZE,
            y * TILE_SIZE
        );
    }

    // Sprites behind foreground
    for (auto& s : impl->sprites)
    {
        if (!s.front)
        {
            drawTile(
                impl->renderer,
                impl->spriteSheet,
                s.tileIndex,
                s.x,
                s.y
            );
        }
    }

    // Foreground tiles
    for (int y = 0; y < MAP_H; y++)
    for (int x = 0; x < MAP_W; x++)
    {
        int tile = impl->foreground[y * MAP_W + x];

        if (tile == -1) continue;

        drawTile(
            impl->renderer,
            impl->tilemap,
            tile,
            x * TILE_SIZE,
            y * TILE_SIZE
        );
    }

    // Sprites in front of foreground
    for (auto& s : impl->sprites)
    {
        if (s.front)
        {
            drawTile(
                impl->renderer,
                impl->spriteSheet,
                s.tileIndex,
                s.x,
                s.y
            );
        }
    }

    SDL_RenderPresent(impl->renderer);
}

bool Interpreter::running() const 
{
    return impl->running;
}

void Interpreter::shutdown()
{
    cleanup();
}

void Interpreter::cleanup()
{
    if (impl->tilemap)
    {
        SDL_DestroyTexture(impl->tilemap);
    }

    if (impl->spriteSheet)
    {
        SDL_DestroyTexture(impl->spriteSheet);
    }

    if (impl->renderer)
    {
        SDL_DestroyRenderer(impl->renderer);
    }

    if (impl->window)
    {
        SDL_DestroyWindow(impl->window);
    }

    if (impl->gamepad)
    {
        SDL_CloseGamepad(impl->gamepad);
    }

    SDL_Quit();
}