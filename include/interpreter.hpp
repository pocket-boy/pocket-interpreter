#pragma once

#include <string>

class Interpreter
{
public:
    Interpreter();
    ~Interpreter();

    bool initialize(int width, int height, const std::string& title);

    bool loadTexture(const std::string& fullPath);

    void tick();
    void render();

    bool running() const;
    void shutdown();

private:
    struct Impl;
    Impl* impl = nullptr;    

    void cleanup();
};