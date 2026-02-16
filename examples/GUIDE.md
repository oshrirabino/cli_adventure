# Examples Guide: Build a Simple Game

This guide explains how to create a minimal playable game for this engine.

## 1. Folder Layout

Create a game root directory with at least:

```
your_game/
  start.level
  endings/
    win.level
    lose.level
```

- `start.level` is the default entry file.
- Endings should be separate endgame levels.

## 2. Level File Format

Each level uses tagged sections.

### Required for choice levels

- `[CONTENT]`
- `[OPTIONS]` with at least one option

Choice level example:

```text
[HEADER]
id: start
title: Prison Cell
ascii_art: ./art/cell.txt

[CONTENT]
You wake up in a locked cell.
There is a tunnel and a gate.

[OPTIONS]
Try the tunnel -> ./tunnel.level
Try the gate -> ./endings/lose.level

[DIRECTIVES]
input_mode: choice
```

Notes:
- Option format: `Text -> relative/path.level`
- Option format with explicit ID (recommended for memory rules):
  - `option_id | Text -> relative/path.level`
- Relative paths are resolved from the current level file location.
- Optional ASCII art in header:
  - `ascii_art: ./art/room.txt`
  - Path is resolved relative to the current level file.

### Memory and conditional options

Use optional sections:

- `[MEMORY]` for level-enter mutations
- `[OPTION_CONDITIONS]` for visibility rules
- `[OPTION_EFFECTS]` for mutations on selected option

Example:

```text
[OPTIONS]
take_axe | Pick up the axe -> ./forest.level
cut_tree | Cut the tree down -> ./clearing.level

[MEMORY]
on_enter add_flag=visited_forest
on_enter set_value=zone:forest

[OPTION_CONDITIONS]
option=cut_tree requires_flag=got_axe

[OPTION_EFFECTS]
option=take_axe add_flag=got_axe
option=take_axe set_value=weapon:axe
```

Supported mutations:

- `add_flag=<flag>`
- `clear_flag=<flag>`
- `set_value=<key>:<value>`
- `erase_value=<key>`

Supported conditions:

- `requires_flag=<flag>`
- `forbids_flag=<flag>`
- `requires_value=<key>:<value>`
- `requires_missing_value=<key>`

### Same Text, different outcomes

To make one visible choice behave differently by memory, define two options with different IDs but the same display text, then gate them with opposite conditions:

```text
[OPTIONS]
fight_magic | Fight demon -> ./endings/win.level
fight_plain | Fight demon -> ./endings/lose.level

[OPTION_CONDITIONS]
option=fight_magic requires_flag=got_magic_sword
option=fight_plain forbids_flag=got_magic_sword
```

This keeps the UI text identical while routing to different targets based on memory.

### Required for endgame levels

- `[DIRECTIVES]`
- `input_mode: endgame`
- `result: victory` or `result: game_over`

Endgame example:

```text
[HEADER]
title: Escape

[CONTENT]
You reach the surface and escape.

[DIRECTIVES]
input_mode: endgame
result: victory
```

If a choice level has no options, the engine treats it as a game-structure error and ends the run.

## 3. Build Your First Simple Game

1. Create `start.level` with two choices.
2. Point one choice to a win endgame level.
3. Point one choice to a lose endgame level.
4. Ensure every target file exists.
5. Run and test both branches.

## 4. Place Your Game in the Library

Put your game folder under the launcher games directory (default: `./games`):

```
games/
  your_game/
    start.level
    ...
```

## 5. Run the Game

From project root:

```bash
./build/cli_adventure
```

Or specify a custom games directory:

```bash
./build/cli_adventure <games_directory>
```

Use a custom theme with default games directory:

```bash
./build/cli_adventure --theme <theme_file>
```

Use custom games directory + custom theme:

```bash
./build/cli_adventure <games_directory> --theme <theme_file>
```

When running in a real terminal:
- Use Up/Down arrow keys to move.
- Press Enter to select.
- Selected option marker defaults to `->`; unselected defaults to `>`.
- After selecting, only the previous scene block is cleared before next scene render.

When stdin/stdout is not a TTY (for scripted runs), numeric fallback is used.

## 6. Customize Style (Colors/Markers/Borders)

Edit `themes/default.theme`:

- `border_line`
- `menu_selected_prefix`
- `menu_unselected_prefix`
- `title_color`, `body_color`, `prompt_color`
- `selected_color`, `unselected_color`
- `victory_color`, `game_over_color`, `error_color`
- `use_color`

You can also run with a custom theme file:

```bash
./build/cli_adventure <games_directory> --theme ./themes/default.theme
```

## 7. Quick Validation Checklist

- Every choice level has at least one option.
- Every option target path is valid.
- If using `[OPTION_CONDITIONS]` or `[OPTION_EFFECTS]`, option IDs referenced by `option=` exist.
- Every ending uses `input_mode: endgame`.
- Every ending has `result: victory` or `result: game_over`.
- Optional: `ascii_art` files exist for levels that reference them.
