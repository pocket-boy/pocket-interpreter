#include "interpreter.hpp"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>

struct Interpreter::Impl
{
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    bool running = false;
};

Interpreter::Interpreter()
{
    impl = new Impl();
}

Interpreter::~Interpreter()
{
    cleanup();
    delete impl;
}

bool Interpreter::initialize(int width, int height, const std::string& title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

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

    impl->running = true;

    return true;
}

bool Interpreter::loadTexture(const std::string& fullPath)
{        
    impl->texture = IMG_LoadTexture(impl->renderer, fullPath.c_str());

    if (!impl->texture)
    {
        std::cerr << "IMG_LoadTexture Error: " << SDL_GetError() << " " << fullPath << std::endl;
        return false;
    }

    return true;
}

void Interpreter::tick()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            impl->running = false;
    }
}

void Interpreter::render()
{
    if (!impl->renderer)
        return;

    SDL_RenderClear(impl->renderer);

    if (impl->texture)
        SDL_RenderTexture(impl->renderer, impl->texture, nullptr, nullptr);

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
    if (impl->texture)
    {
        SDL_DestroyTexture(impl->texture);
        impl->texture = nullptr;
    }

    if (impl->renderer)
    {
        SDL_DestroyRenderer(impl->renderer);
        impl->renderer = nullptr;
    }

    if (impl->window)
    {
        SDL_DestroyWindow(impl->window);
        impl->window = nullptr;
    }

    SDL_Quit();
}