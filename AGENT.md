# AGENT.md

You are an C++ coding agent working inside this repository.

This file defines **mandatory rules and workflow**.  
You must read and follow them before starting any task.

---

## 1. Required Reading (Always)

Before making **any** change, you MUST read:

1. `architecture.md`  
   - This file defines the system architecture, boundaries, and design principles.
   - Do not violate architectural decisions unless explicitly instructed.

2. The current codebase relevant to the task.
   - Never assume behavior without verifying it in code.

3. Work in src directory for C++ files.

If any information is missing or unclear, stop and report it.

---

## 2. Core Principles

All code you write must be:

- **Readable** – clear naming, simple control flow, no clever tricks.
- **Extensible** – easy to modify without rewriting existing logic.
- **Minimal** – no unnecessary abstractions or premature generalization.
- **Consistent** – follow existing patterns, style, and conventions.
- **Safe to change** – avoid tight coupling and hidden side effects.

Prefer clarity over brevity.

---

## 3. Architectural Rules

- Respect module boundaries defined in `architecture.md`.
- Do not introduce circular dependencies.
- Do not bypass layers.
- New concepts must have a clear, single responsibility.
- If you introduce a new abstraction, explain why it is needed.

If a requested change conflicts with the architecture:
- Do NOT implement it silently.
- Report the conflict and propose alternatives.

---

## 4. Coding Rules

- Functions and classes must do **one thing**.
- Names must reflect intent, not implementation details.
- Avoid large functions; refactor when logic grows.
- Avoid hidden state and implicit behavior.
- Prefer pure functions where possible.

When modifying existing code:
- Preserve public APIs unless explicitly told otherwise.
- Avoid breaking changes.
- Refactor only when it improves clarity or extensibility.

---

## 5. Change Tracking (Mandatory)

You MUST document your work in: CHANGES.md

