# CLI Adventure Engine

A C++ terminal game engine for data-driven text adventures.

The engine is separated from story content:

- Engine code lives in `src/`
- Games are authored as `.level` files in folders under `games/` (or another games directory)

## Features

- Launcher with persistent sessions (`Play Game`, `Settings`, `Validate Games`, `Exit`)
- Choice-based levels
- Free-text input levels
- Endgame levels (`victory` / `game_over`)
- Theme-based terminal styling
- Optional ASCII art per level
- Memory system (flags + key/value) for conditional branching and state effects
- Built-in game validator for creators

## Quick Start

1. Build:

```bash
cmake -S . -B build
cmake --build build
```

2. Run launcher:

```bash
./build/cli_adventure
```

Optional:

- Custom games directory:

```bash
./build/cli_adventure <games_directory>
```

- Custom theme:

```bash
./build/cli_adventure --theme <theme_file>
./build/cli_adventure <games_directory> --theme <theme_file>
```

## How To Add Games

Create a folder under `games/`:

```text
games/
  my_game/
    start.level
    endings/
      win.level
      lose.level
```

`start.level` is required (entry file).

### Minimal Choice Level

```text
[HEADER]
id: start
title: My First Room

[CONTENT]
You stand before two doors.

[OPTIONS]
go_left | Open the left door -> ./left.level
go_right | Open the right door -> ./right.level

[DIRECTIVES]
input_mode: choice
```

### Minimal Endgame Level

```text
[HEADER]
title: Victory

[CONTENT]
You escaped.

[DIRECTIVES]
input_mode: endgame
result: victory
```

### Minimal Input Level

```text
[HEADER]
title: Riddle

[CONTENT]
A voice asks for the password.

[DIRECTIVES]
input_mode: input
input_prompt: Type password:
input_match: exact
input_case_sensitive: false

[INPUT_RULES]
say_friend | friend -> ./next.level
```

## Memory And Conditional Branching

Use memory to unlock options/rules and apply effects.

Supported sections:

- `[MEMORY]` (on-enter mutations)
- `[OPTION_CONDITIONS]` (visibility gates)
- `[OPTION_EFFECTS]` (mutations on chosen option or matched input rule)

Supported mutations:

- `add_flag=<flag>`
- `clear_flag=<flag>`
- `set_value=<key>:<value>`
- `erase_value=<key>`

Example:

```text
[OPTION_CONDITIONS]
option=open_gate requires_flag=got_key

[OPTION_EFFECTS]
option=take_key add_flag=got_key
```

## Validate A Game

From launcher:

1. Choose `Validate Games`
2. Select a game
3. Review reported issues (missing targets, invalid directives, unknown IDs, parse failures)

## Documentation

- `GAME_SETUP.md` - setup and runtime behavior
- `examples/GUIDE.md` - format guide with examples
- `themes/README.md` - theme file reference
- `architecture.md` - architecture notes
