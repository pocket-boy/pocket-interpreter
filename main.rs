use compiler_core::{
    backend::{Backend, Environ},
    compiler::Compiler,
    parser::Parser,
};

const SAMPLE_FILE_1: &str = "
local let sprite := \"assets://sprite.png\"

local let x: int = 50
local let y: int = 30

def step(window: Window, x: int, y: int) -> unit:
    draw(window, 32, x, y)

def render(window: Window, _: Input) -> unit:
    step(window, x, y)
    step(window, y, x)
";
const SAMPLE_FILE: &str = "local let sprite := \"assets://sprite.png\"\n\nlocal let x: int = 50\nlocal let y: int = 30\n\ndef step(window: Window, x: int, y: int) -> unit:\n\tdraw(window, 32, x, y)\n\ndef render(window: Window, _: Input) -> unit:\n\tstep(window, x, y)\n\tstep(window, y, x)";

pub struct EnvironStub;

impl Environ for EnvironStub {
    fn action_one(&mut self, value: u32) {
        println!("[ONE]: {value}");
    }

    fn action_two(&mut self, value: u32) {
        println!("[TWO]: {value}");
    }

    fn draw_tile(&mut self, tile: u32, x: u32, y: u32) {
        println!("[DRAW]: {tile}, {x}, {y}");
    }
}

fn main() {
    // println!("{:#?}", Parser::file(SAMPLE_FILE));
    let mut compiler = Compiler::default();

    compiler.load_module("main", SAMPLE_FILE);

    let mut backend = Backend::new(compiler, EnvironStub);

    backend.render();
}
