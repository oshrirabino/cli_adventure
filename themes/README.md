# Theme File Guide

Theme files are simple `key = value` text files.

Default file:

- `themes/default.theme`

Run with a custom theme:

```bash
./build/cli_adventure --theme ./themes/default.theme
./build/cli_adventure games --theme ./themes/default.theme
```

## Supported Keys

- `use_color`
- `border_line`
- `menu_selected_prefix`
- `menu_unselected_prefix`
- `title_color`
- `body_color`
- `prompt_color`
- `selected_color`
- `unselected_color`
- `victory_color`
- `game_over_color`
- `error_color`

## Example

```text
use_color = true
border_line = ************************************************************
menu_selected_prefix = ->
menu_unselected_prefix = >
title_color = bright_cyan
body_color = default
prompt_color = bright_yellow
selected_color = bright_green
unselected_color = default
victory_color = bright_green
game_over_color = bright_red
error_color = bright_red
```

## Color Names

- `default`
- `black`, `red`, `green`, `yellow`, `blue`, `magenta`, `cyan`, `white`
- `bright_black`, `bright_red`, `bright_green`, `bright_yellow`
- `bright_blue`, `bright_magenta`, `bright_cyan`, `bright_white`

Unknown color names fall back to plain text output for that field.

These same color names are also valid in ASCII art line tags:

- `[color=<name>]...`
- `[color:<name>]...`
- `[default_color=<name>]` (whole-file default color directive)
- `[art_color=<name>]` (alias for whole-file default)
