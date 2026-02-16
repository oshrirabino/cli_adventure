## 2026-02-16

- Implemented stage 1 project scaffold:
  - Added CMake build with `adventure_engine` library and `cli_adventure` executable.
  - Created module layout under `src/`: `context`, `engine`, `levels`, `parser`, `fs`, `ui`.
  - Added executable entrypoint in `src/main.cpp`.

- Implemented stage 2 core contracts:
  - Added `GameContext` state model for shared runtime state.
  - Added `ILevel` interface with `render` and `execute` contract.
  - Added `Engine` loop shell and `NullLevel` fallback behavior.
  - Added `tests/context_tests.cpp` to verify context state updates.

- Implemented stage 3 tag parser:
  - Added `ParsedLevelData` model and `TagParser` with resilient tag-based parsing.
  - Supported sections: `[HEADER]`, `[CONTENT]`, `[OPTIONS]`, `[DIRECTIVES]`.
  - Supported option separators `->` and `=>`.
  - Added `tests/parser_tests.cpp` covering happy path, malformed lines, and missing sections.
  - Added `examples/tag_format_example.level` outside `src/` as initial authoring format sample.

- Refactored shared context to generic game memory:
  - Removed domain-specific `player_health` and inventory APIs from `GameContext`.
  - Added generic memory map (`string -> string`) and memory flag set (`string`).
  - Added APIs for set/get/erase key-values and set/clear/has flags.
  - Updated context tests to validate generic memory behavior.
  - Updated architecture notes and example format to reflect memory-driven design.

- Added interactive terminal level flow:
  - Introduced `ChoiceLevel` with robust numeric input validation and terminal rendering.
  - Added `TerminalLevelFactory` to create concrete levels from parsed data (`input_mode` based).
  - Refactored `Engine` to parse/build/execute levels from files each turn.
  - Added level-to-level transitions by resolving relative paths against the current level directory.
  - Added terminal directive handling (`end: victory` / `end: game_over`) in choice levels with no options.
  - Updated `src/main.cpp` to boot from a playable campaign entry level.

- Added coverage for interactive flow:
  - `tests/choice_level_tests.cpp` for invalid/valid choice input and victory terminal directive.
  - `tests/engine_flow_tests.cpp` for end-to-end parse/factory/navigation flow.

- Added playable content examples:
  - `examples/campaign/start.level`
  - `examples/campaign/gate.level`
  - `examples/campaign/tunnels/entry.level`

- Separated endgame handling from choice levels:
  - Added dedicated `EndGameLevel` (`input_mode: endgame`) with `result: victory|game_over`.
  - Updated `TerminalLevelFactory` to instantiate `EndGameLevel` for endgame content.
  - Removed terminal-ending behavior from `ChoiceLevel`.
  - `ChoiceLevel` without options now ends session with a clear `[Structure Error]` message.
  - Updated campaign examples and tests to route endings through explicit endgame levels.

- Updated CLI runtime boot flow:
  - `main` now requires a game root path argument and optional entry level relative path.
  - Default entry level is `start.level` under the provided game root.
  - Added path validation and user-facing usage/errors for invalid game structure.
  - Added `GAME_SETUP.md` with minimal structure rules and run commands.

- Added `examples/GUIDE.md` with step-by-step instructions for creating a simple playable game.

- Added interactive CLI navigation and game picker:
  - Added `src/ui/terminal_menu.*` for reusable terminal menu selection.
  - Interactive mode now uses Up/Down keys + Enter when running in a real TTY.
  - Automatic numeric fallback remains for non-interactive input (tests/pipes).
  - Choice levels now use the terminal menu mechanism instead of number-only prompt flow.
  - `main` now scans a games directory (`./games` by default, or custom arg), lets player pick a game, then starts that game's `start.level`.
  - Updated setup guides to document the games-library launcher flow and controls.

- Added style/theming separation and optional ASCII art:
  - Introduced `Theme` config (`src/ui/theme.*`) and `Renderer` (`src/ui/renderer.*`).
  - Added external style file support (`themes/default.theme`) for easy visual customization.
  - Added optional `--theme <theme_file>` CLI flag for launcher/theme selection.
  - Updated menu markers to theme-driven prefixes (`->` selected, `>` unselected by default).
  - Moved level framing/status rendering into renderer (separate from gameplay logic).
  - Added optional `ascii_art` header field in level format; art path resolves relative to the level file.

- Added scene-to-scene terminal clearing in interactive mode:
  - After selecting an option, the previous level block is cleared before drawing the next level.
  - Clearing only affects the last rendered scene/menu region, not the entire terminal buffer.

- Updated guides/setup docs to current runtime behavior:
  - Expanded `GAME_SETUP.md` with current CLI patterns, controls, and scene-clearing behavior.
  - Updated `examples/GUIDE.md` with current `ascii_art` usage, theme CLI options, and scene-clearing notes.
  - Added `themes/README.md` documenting theme file keys and supported color names.

- Refinement and cleanup pass for first stable engine version:
  - Removed obsolete `NullLevel` implementation and build wiring (dead code path).
  - Replaced internal raw renderer pointers with references in level/factory classes.
  - Simplified `main` argument parsing into a dedicated parser helper for maintainability.
  - Cleaned renderer API (`render_scene` now void) and fixed scene line accounting when ASCII art is missing.
  - Normalized default unselected menu marker to `>` in theme defaults.

- Addressed follow-up review findings:
  - Engine now catches level load/execute exceptions and reports a structure error instead of terminating abruptly.
  - Engine now guards against levels that neither transition nor terminate, preventing accidental infinite rerender loops.
  - Removed choice-scene clear magic number by returning menu render metadata from `pick_option`.
  - Improved option delimiter parsing to split on the earliest of `->` or `=>` when both are present.
  - Added raw-mode signal handling to restore terminal settings on signals (e.g., Ctrl+C) during interactive input.
  - Added regression tests for missing next-level file handling and option delimiter precedence.

- Change render to render the ASCII art after the lwvwl content.