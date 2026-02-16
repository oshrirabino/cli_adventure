# First Game Setup

## Required Minimal Structure

Each game folder must contain at least:

- `start.level` (entry level)
- At least one endgame level using `input_mode: endgame`

Example:

```
my_game/
  start.level
  endings/
    win.level
    lose.level
```

## Level File Format (Current)

Supported sections:

- `[HEADER]`
- `[CONTENT]`
- `[OPTIONS]`
- `[DIRECTIVES]`

## Required Level Rules

- Choice level:
  - Has `[OPTIONS]` with at least one option.
  - If empty/missing options, engine stops with a structure error.
  - Optional header field:
    - `ascii_art: ./art/name.txt` (relative to current level file)
- Endgame level:
  - `[DIRECTIVES]` must include:
    - `input_mode: endgame`
    - `result: victory` or `result: game_over`

## Game Library Layout

The launcher scans a games directory and lists each subdirectory that contains `start.level`.

Example:

```
games/
  the_iron_key/
    start.level
    ...
  my_second_game/
    start.level
    ...
```

## Run Command

Default games directory (`./games`):

```bash
./build/cli_adventure
```

Custom games directory:

```bash
./build/cli_adventure <games_directory>
```

Default games directory + custom theme:

```bash
./build/cli_adventure --theme <theme_file>
```

Custom theme:

```bash
./build/cli_adventure <games_directory> --theme <theme_file>
```

Example with current games folder:

```bash
./build/cli_adventure games
```

## Controls

In a real terminal (TTY):

- Up/Down arrows move selection.
- Enter confirms selection.
- Selected option marker defaults to `->`.
- Unselected option marker defaults to `>`.

In non-interactive mode (pipes/tests):

- Numeric fallback is used (`1`, `2`, ...).

## Scene Clearing Behavior

- In interactive mode, after a choice is selected, only the previous scene block is cleared.
- The full terminal history is not wiped.
