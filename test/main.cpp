#include "interpreter.hpp"
#include <filesystem>
#include <string>
#include <iostream>

int main()
{
    Interpreter interpreter;

    if (!interpreter.initialize(768, 768, "Interpreter - Asset Loading Test"))
        return 0;

    std::filesystem::path assets = ASSETS_DIR;

    auto tilemap = assets / "test.png";
    auto sprites = assets / "test2.png";

     if (!interpreter.loadTilemap(tilemap.string()))
        return 1;

    if (!interpreter.loadSpriteSheet(sprites.string()))
        return 1;

    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 32; x++)
        {
            int tile = (x + y) % 24;

            interpreter.setBackgroundTile(x, y, tile);
        }
    }

    for (int y = 6; y < 20; y++)
    {
        for (int x = 6; x < 20; x++)
        {
            interpreter.setForegroundTile(x, y, 32);
        }
    }

    Interpreter::Sprite s1;
    s1.x = 4 * 8;
    s1.y = 8 * 8;
    s1.tileIndex = 1;
    s1.front = false;

    interpreter.addSprite(s1);

    Interpreter::Sprite s2;
    s2.x = 6 * 8;
    s2.y = 6 * 8;
    s2.tileIndex = 0;
    s2.front = true;

    interpreter.addSprite(s2);

    // Main loop.
    while (interpreter.running())
    {
        interpreter.tick();
        interpreter.render();
    }

    interpreter.shutdown();

    return 0;
}