#include "backend.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"
#include <filesystem>
#include <fstream>
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

extern "C" void load_tile(ExternEnviron *self, const char *tiles) {
  interpreter.loadTilemap(tiles);
}

const char *GAME = R"(
local let MOVE_TIME := 2

local let TEXT_TIME := 2

local var x := 5

local var y := 5

local var move_tick := MOVE_TIME

local var first := true

local let num_places := 3

local var places_x := [3]

local var places_y := [3]

local var current := 0

local let text := [11]

local var text_tick := TEXT_TIME

local var text_count := 0

def render(window: Window, input: Input) -> unit:

  if first:
    first = false

    places_x = Array.set(places_x, 0, 20)
    places_x = Array.set(places_x, 1, 10)
    places_x = Array.set(places_x, 2, 15)

    places_y = Array.set(places_y, 0, 20)
    places_y = Array.set(places_y, 1, 10)
    places_y = Array.set(places_y, 2, 15)

    text = Array.set(text, 0, 9)
    text = Array.set(text, 1, 10)
    text = Array.set(text, 2, 11)
    text = Array.set(text, 3, 0)
    text = Array.set(text, 4, 12)
    text = Array.set(text, 5, 10)
    text = Array.set(text, 6, 13)
    text = Array.set(text, 7, 0)
    text = Array.set(text, 8, 14)
    text = Array.set(text, 9, 13)
    text = Array.set(text, 10, 15)

  if ((move_tick == 0) && Input.pressed(input, Input.LEFT)) && (x > 0):
    x = x - 1
    move_tick = MOVE_TIME

  if ((move_tick == 0) && Input.pressed(input, Input.RIGHT)) && (x < 31):
    x = x + 1
    move_tick = MOVE_TIME

  if ((move_tick == 0) && Input.pressed(input, Input.UP)) && (y > 0):
    y = y - 1
    move_tick = MOVE_TIME

  if ((move_tick == 0) && Input.pressed(input, Input.DOWN)) && (y < 31):
    y = y + 1
    move_tick = MOVE_TIME

  if move_tick > 0:
    move_tick = move_tick - 1

  Window.draw(window, 1, x, y)

  if current < num_places:
    let curr_x := Array.get(places_x, current)
    let curr_y := Array.get(places_y, current)

    Window.draw(window, 2, curr_x, curr_y)

    if (x == curr_x) && (y == curr_y):
      current = current + 1

  if current >= num_places:
    if text_tick == 0:
      text_tick = TEXT_TIME

      if text_count <= 10:
        text_count = text_count + 1

    var current := 0
    while current < text_count:
      Window.draw(window, Array.get(text, current), 3 + current, 3)
      current = current + 1

    text_tick = text_tick - 1
)";

int main(int argc, const char **argv) {

  if (argc < 2) {
    return 1;
  }
  std::string code;
  std::ifstream fin(argv[1]);
  if (fin) {
    std::ostringstream file_content;
    file_content << fin.rdbuf();
    code = file_content.str();

  } else {
    std::cerr << "Main Script not found!\n";
    return 1;
  }

  if (!interpreter.initialize(768, 768, "Interpreter - Asset Loading Test"))
    return 0;

  std::filesystem::path assets = ASSETS_DIR;

  auto tilemap = assets / "test.png";
  auto sprites = assets / "test2.png";

  if (!interpreter.loadTilemap(tilemap.string()))
    return 1;

  if (!interpreter.loadSpriteSheet(sprites.string()))
    return 1;

  ExternEnviron environ = {.action_one = action_one,
                           .action_two = action_two,
                           .draw_tile = draw_tile,
                           .load_tile = load_tile};

  CompilerHandle compiler;

  compiler.load_module("main", code.c_str());

  BackendHandle backend(compiler, environ);

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
