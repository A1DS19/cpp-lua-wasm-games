# JPEngine

[![Linux Build (jpengine-2d)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-linux-2d.yml/badge.svg?branch=main)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-linux-2d.yml)
[![WASM Build (jpengine-2d)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-wasm-2d.yml/badge.svg?branch=main)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-wasm-2d.yml)
[![Linux Build (jpengine-3d)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-linux-3d.yml/badge.svg?branch=main)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/build-linux-3d.yml)
[![CodeQL](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/codeql.yml/badge.svg?branch=main)](https://github.com/A1DS19/cpp-lua-wasm-games/actions/workflows/codeql.yml)

A collection of games built with **jpengine-2d**, a custom 2D game engine in C++ with Lua scripting and WebAssembly support.

## Engine

- [jpengine-2d](./jpengine-2d/README.md) — engine source, build instructions, and Lua API reference

## Games

| Game                                          | Description      |
| --------------------------------------------- | ---------------- |
| [platformer](./jpengine-2d/games/platformer/) | Jump across platforms, collect every coin, and reach the flag to win |
| [tetris](./jpengine-2d/games/tetris/)         | Classic Tetris with scoring, levels, and increasing speed |

## Screenshots

### Platformer

![Platformer gameplay](./jpengine-2d/games/platformer/assets/screenshots/jumpy-coin.png)

### Tetris

![Tetris gameplay](./jpengine-2d/games/tetris/assets/screenshots/game-1.png)

## Development

This repo uses [pre-commit](https://pre-commit.com/) to run formatting and lint checks (clang-format, actionlint, hygiene hooks) before each commit. One-time setup:

```sh
sudo dnf install pre-commit   # or: pip install pre-commit / brew install pre-commit
pre-commit install
```

After install, hooks run automatically on `git commit`. To run them against the whole repo on demand:

```sh
pre-commit run --all-files
```
