#include "backend.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"
#include <filesystem>
#include <iostream>
#include <string>

static Interpreter interpreter;

extern "C" void action_one(ExternEnviron *self, uint32_t value) {
  std::cout << "[ONE]: " << value << "\n";
}

extern "C" void action_two(ExternEnviron *self, uint32_t value) {
  std::cout << "[TWO]: " << value << "\n";
}

extern "C" void draw_tile(ExternEnviron *self, uint32_t tile, uint32_t x,
                          uint32_t y) {
  interpreter.setForegroundTile(x, y, tile);
}

const char *GAME =
    "def render(window: Window, input: Input) -> unit:\n\tWindow.draw(window, "
    "2, 7, "
    "5)\n\tif Input.pressed(input, Input.LEFT):\n\t\tWindow.draw(window, 1, 5, "
    "5)\n\tif "
    "Input.pressed(input, Input.DOWN):\n\t\tWindow.draw(window, 1, 6, 5)\n\tif "
    "Input.pressed(input, Input.RIGHT):\n\t\tWindow.draw(window, 1, 7, "
    "5)\n\tif "
    "Input.pressed(input, Input.UP):\n\t\tWindow.draw(window, 3, 7, 4)";

int main(void) {

  if (!interpreter.initialize(768, 768, "Interpreter - Asset Loading Test"))
    return 0;

  std::filesystem::path assets = ASSETS_DIR;

  auto tilemap = assets / "littleguy.png";
  auto sprites = assets / "test2.png";

  if (!interpreter.loadTilemap(tilemap.string()))
    return 1;

  if (!interpreter.loadSpriteSheet(sprites.string()))
    return 1;

  ExternEnviron environ = {.action_one = action_one,
                           .action_two = action_two,
                           .draw_tile = draw_tile};

  CompilerHandle compiler;

  compiler.load_module("main", GAME);

  BackendHandle backend(std::move(compiler), environ);

  // for (int y = 0; y < 32; y++) {
  //   for (int x = 0; x < 32; x++) {
  //     int tile = (x + y) % 24;
  //
  //     interpreter.setBackgroundTile(x, y, tile);
  //   }
  // }
  //
  // for (int y = 6; y < 20; y++) {
  //   for (int x = 6; x < 20; x++) {
  //     interpreter.setForegroundTile(x, y, 32);
  //   }
  // }

  // Interpreter::Sprite s1;
  // s1.x = 4 * 8;
  // s1.y = 8 * 8;
  // s1.tileIndex = 1;
  // s1.front = false;
  //
  // interpreter.addSprite(s1);
  //
  // Interpreter::Sprite s2;
  // s2.x = 6 * 8;
  // s2.y = 6 * 8;
  // s2.tileIndex = 0;
  // s2.front = true;
  //
  // interpreter.addSprite(s2);

  // Main loop.
  while (interpreter.running()) {
    interpreter.begin();

    interpreter.clear();

    interpreter.process();

    backend.render(interpreter.input());

    interpreter.delay();

    interpreter.render();
  }

  interpreter.shutdown();

  return 0;
}
