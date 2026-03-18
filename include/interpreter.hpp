#pragma once

#include <string>
#include <vector>

class Interpreter
{
public:
    Interpreter();
    ~Interpreter();

    struct Sprite
    {
        int x;
        int y;
        int tileIndex;
        bool front;
    };

    bool initialize(int width, int height, const std::string& title);

    bool loadTilemap(const std::string& path);
    bool loadSpriteSheet(const std::string& path);

    void setBackgroundTile(int x, int y, int tile);
    void setForegroundTile(int x, int y, int tile);

    void addSprite(const Sprite& sprite);
    void clearSprites();

    void tick();
    void render();

    bool running() const;
    void shutdown();

private:
    struct Impl;
    Impl* impl = nullptr;

    void cleanup();
};