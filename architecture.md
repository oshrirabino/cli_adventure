Architecture: Interactive Terminal Game Engine
1. Project Vision & Use Case
We are building a generic, data-driven game engine in C++ designed to run interactive text adventures in a terminal environment.

The Goal: To separate the Engine (logic) from the Content (stories).

The Engine: A robust executable that knows how to parse files, render text/ASCII art, and manage game state.

The Content: A directory tree of simple text files created by users. Users can structure their games using folders (acting as chapters or zones) and files (acting as individual levels/scenes).

The system is designed to be modular and expandable, allowing for different types of gameplay (multiple choice, puzzle input) and visual enhancements (animations) without modifying the core engine loop.

2. The Gameplay Flow
The engine operates on a continuous loop until a "Game Over" or "Victory" state is reached.

Load: The engine identifies the current level file based on the game state.

Parse & Build: The file is read, and a Level Object is constructed in memory.

Render: The level displays its content (Text, ASCII Art, Animations) to the terminal.

Input: The player interacts with the level (selecting an option or typing a command).

State Update: The level logic processes the input and modifies the shared Game Context (e.g., writing memory keys/flags or requesting the next level file).

Transition: The engine resolves the path to the next level (handling directory changes if needed) and repeats the loop.

3. Data Format Strategy
To ensure "anyone can write a story," we utilize a custom Tag-Based Text Format instead of strict formats like JSON or YAML.

Concept: The format mimics a standard document with labeled sections (e.g., [HEADER], [CONTENT], [OPTIONS]).

Flexibility: The parser is designed to be resilient. It reads line-by-line and switches modes based on tags. This allows creators to write multi-line story text naturally without worrying about escape characters or strict indentation.

Directives: The files contain instructions for the engine, such as input mode, what choices are available, and where those choices lead (using relative file paths).

Header Visuals: Levels may include an optional `ascii_art` field in `[HEADER]`. When present, the renderer loads that file relative to the level file location and prints it before the content text.

Memory Hooks: The format can include memory declarations (for example, a `[MEMORY]` section). These declarations are interpreted by a memory-related decorator layer rather than hardcoded in the base level loop.

4. System Architecture
A. The Engine Core (The Conductor)
The Engine is the central controller. It initializes the system, manages the main game loop, and coordinates the communication between the File System, the Level Factory, and the User Interface. It is responsible for the "lifecycle" of the application.

B. State Management (The Context Pattern)
We avoid passing specific parameters into every function. Instead, we use a Shared Context Object.

Role: This object acts as the "Single Source of Truth" for the current game session.

Contents: It holds:
- System state (Current Directory, Next Level Request, Game Over/Victory flags).
- Generic game memory:
  - Key/Value map (`string -> string`) for values like counters, states, item ownership markers, quest stages.
  - String set for boolean-like flags/events (e.g., `door.opened`, `boss.defeated`).

Usage: The Engine passes this Context to the active Level. The Level reads from it to render the scene and writes to it to determine the outcome of the turn.

C. Level Design (Polymorphism)
All gameplay scenarios are treated uniformly by the Engine through a common Interface.

The Contract: Every Level must have a method to Render itself and a method to Execute logic.

Abstraction: The Engine does not know if it is running a "Combat Encounter," a "Dialogue Scene," or a "Puzzle." It simply calls the interface methods.

D. Navigation & File System
The system supports a hierarchical file structure.

Relative Paths: Levels reference other levels using relative paths (e.g., ../dungeon/cell.txt).

Path Resolution: The Engine maintains the "Current Working Directory" of the active story. When a Level requests a jump, the Engine resolves the new absolute path, allowing for complex, multi-folder story campaigns.

E. Presentation Layer (Theme + Renderer)
Rendering style is separated from gameplay logic.

Theme: Visual tokens (colors, border line, option prefixes) are loaded from an external theme file.

Renderer: Levels delegate layout and status rendering to a renderer that consumes the active theme.

Benefit: Visual restyling can happen without changing gameplay behavior.

5. Extensibility & Design Patterns
A. The Factory Pattern (Creation)
We encapsulate the complexity of creating levels inside a Level Factory.

Role: The Factory reads the parsed data from a file and decides which C++ class to instantiate.

Benefit: This centralizes dependency injection and object creation logic, keeping the Engine code clean.

B. The Decorator Pattern (Expansion)
To add features without creating a messy inheritance tree (e.g., AnimatedStoryLevel, AnimatedPuzzleLevel), we use Decorators.

Concept: We separate "Core Logic" (Input/Choice) from "Features" (Animation/Memory effects).

Usage: A base Level object is created (handling the logic). If the file specifies an animation or special effect, the Factory wraps the Level object inside a Decorator.

Result: The Decorator hijacks the Render or Execute call to add its behavior (like playing an ASCII animation or applying memory writes declared in content) before passing control to the underlying Level.

C. Strategy Pattern (Input Handling)
Different levels require different input methods. We use the Strategy pattern (via subclassing the base Level) to swap out input mechanisms.

Choice Strategy: Restricts input to valid integers maps to options.

Free Input Strategy: Accepts raw strings and parses them for keywords (for riddles/puzzles).

EndGame Strategy: Dedicated terminal level type that ends the run with `victory` or `game_over`. Choice levels must always define options; a choice level without options is treated as a game-structure error.
