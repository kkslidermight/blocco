# AI Contributor Guide (copilot-instructions)

Concise project-specific rules so an AI agent can be productive immediately.

## Big Picture
Prototype of a from-scratch C++23 Vulkan + SDL3 first‑person sandbox (no external game engine). Current repo is an initial scaffold: core systems are stubbed and must be expanded to meet the detailed spec in `.github/prompts/cppgame.prompt.md` (treat that file as the authoritative requirements list). Implementation should grow iteratively with small, logically separated commits.

## Directory Layout & Roles
- `CMakeLists.txt`: Root build; pulls in modular cmake helpers then adds `shaders/`, `src/`, `tests/`.
- `cmake/` modules:
  - `PreventInSourceBuilds.cmake`: Safety check.
  - `CompilerWarnings.cmake`: Central warning flags; use `set_project_warnings(<target>)` for every new target.
  - `FetchSDL3.cmake`: FetchContent of SDL3. (Long term: permit system package discovery before fallback.)
  - `VulkanHelpers.cmake`: `compile_glsl()` helper that invokes `glslc` (from the Vulkan SDK tools) to produce SPIR-V.
  - `Options.cmake`: Feature toggles (`BLOCCO_ENABLE_MARCH_NATIVE`, `BLOCCO_ENABLE_VALIDATION`, `BLOCCO_HEADLESS`). Respect these instead of inventing new ad‑hoc options.
- `shaders/`: GLSL sources compiled at build time. Add new shader filenames to `GLSL_SOURCES` in `shaders/CMakeLists.txt` so they become part of the `blocco_shaders` custom target.
- `src/`: Engine code. Single library target `blocco_engine` plus executables `blocco` (interactive) and `blocco_headless` (offscreen capture). Add new subsystem source files to `src/CMakeLists.txt` (keep list alphabetized when you expand it to reduce merge noise).
- `tests/`: Currently unit tests only (`unit_tests` target). Follow the existing simple pattern (one `main()` per test file using `assert`). When integration tests are added, prefer a separate target (e.g. `integration_tests`) rather than overloading unit tests.
- `scripts/`: fish shell automation. Maintain fish syntax; do not switch to bash. Extend these rather than duplicating logic inside CI.
- `debug/captures/`: Runtime frame dumps (future real PNG + thumbnail) — ensure scripts and code write here.
- `env/`: Generated environment / system audit JSON.
- `.github/`: CI workflow plus prompt spec and these instructions.

## Build & Tooling Conventions
- Require CMake >= 3.25 and Ninja generator. Keep new logic in modular `cmake/*.cmake` files when it generalizes.
- Always add new executable / library targets before referencing them to avoid ordering pitfalls; apply `set_project_warnings()` to each.
- Shaders: rely on `compile_glsl()`; do NOT embed raw GLSL compilation logic elsewhere. Keep GLSL version at `#version 450` unless a higher version is justified; update all shaders consistently if you raise it.
- Validation layers: wrap Vulkan debug / validation enabling behind `BLOCCO_ENABLE_VALIDATION` (default ON). Do not gate core correctness logic behind this flag—only diagnostics.
- Avoid introducing heavyweight abstraction layers or third-party scene/physics libs. Implement minimal purpose-built code.

## Runtime Architecture (Intended Evolution)
- `Engine` orchestrates subsystems: initialization, main loop, per-frame update, render, shutdown. Add structured order (input -> physics -> camera -> renderer) as you implement.
- `Renderer` will own: SDL window/surface selection (Wayland first, X11 fallback), Vulkan instance/device/queues, swapchain management, pipeline objects, descriptor sets, synchronization primitives, and frame capture.
- `InputSystem` will wrap SDL events for keyboard/mouse (relative motion + pointer lock on Wayland). Provide state queries the update step can consume.
- `Camera` will supply view/projection matrices (add functions for clamped pitch, yaw wrap, movement vectors). Keep math lightweight and testable (expand `math.hpp` as needed; each new math function gets unit tests).
- `Config` will handle loading/saving a user config file (choose a simple format like TOML/JSON; once chosen remain consistent). Provide getters; avoid global variables.
- `Collision` / physics: Simple AABB vs world boundaries; keep deterministic and test with edge cases (tunneling avoidance via discrete step checks or conservative advancement).
- `Capture` utilities: Real PNG writing (likely via stb_image_write single-header embedded locally) replacing current dummy placeholder.
- `logging.*`: Expand to structured logging if needed, but keep stdout flushing on important events.

## Tests
- Keep unit tests deterministic and fast (<1s). Add tests for every new math and collision function. Use plain `assert` for now; if complexity grows consider lightweight header-only test framework added via `cmake/`.
- Future integration tests: headless run (use `blocco_headless`) for N frames with validation layers ON and produce a non-empty PNG. Guard with an option if they become slow.

## Scripts & Automation
- `scripts/setup.fish`: Expand dependency probe list; when adding packages, avoid duplicates; output a single pacman command.
- `scripts/build.fish`: Accept optional build type (default Debug). Consider adding CMake presets later (`CMakePresets.json`).
- `scripts/run.fish`: Add flags like `--headless-capture N` mapping to `blocco_headless`.
- `scripts/diagnose.fish`: Enrich JSON with GPU, Vulkan instance/device properties, SDL video driver, package versions. Keep output stable (keys sorted) for diffability.
- `scripts/commit.fish`: Before committing, run build + tests unless `--allow-broken`. Also update `PROGRESS.md` and prune completed items from `TODO.md` (when you implement that logic, keep diff minimal). Enforce conventional commit style (`type: scope: message`).

## Source Style & Patterns
- Header/impl pairs; keep declarations minimal. Avoid inline heavy logic in headers to keep rebuild times lower as project grows.
- Prefer explicit initialization; no hidden globals. If singletons become necessary (e.g., instance of `Renderer`), inject references instead.
- When adding Vulkan objects, encapsulate RAII cleanup in small structs/classes; no raw `vkDestroy*` calls scattered across unrelated files.
- Keep frame loop code free of direct SDL or Vulkan calls once abstractions exist—route through subsystems.

## Adding Features (Example Workflow)
1. Create new component (e.g., `ubo.hpp/ubo.cpp`) and add to `src/CMakeLists.txt` list.
2. Add targeted unit tests in `tests/` (e.g., `test_camera.cpp`).
3. Update `TODO.md` (remove implemented line, add any follow-up subtasks) and append a concise entry to `PROGRESS.md`.
4. Run `scripts/build.fish` then `scripts/test.fish` locally; fix issues.
5. Commit via `scripts/commit.fish "feat(renderer): add uniform buffer setup"`.

## Do / Don’t
- DO: Reference `.github/prompts/cppgame.prompt.md` before planning changes; keep alignment with spec items.
- DO: Keep commits small and labeled (`feat:`, `fix:`, `chore:`, `refactor:`, `test:`, `docs:`).
- DO: Introduce new CMake options only if they control reproducible build variants.
- DON’T: Pull large helper libraries where a few dozen lines suffice.
- DON’T: Hardcode ephemeral absolute paths (keep relative to project root for captures / env snapshots).

## Gaps To Fill (High Priority Next Steps)
These are not aspirations—they are required by the existing spec but unimplemented:
- Vulkan instance/device/swapchain + validation & resize.
- Camera math (view/proj generation, pitch clamp).
- Input event processing (WASD, mouse relative mode, ESC quit).
- Basic room geometry + colored faces + text labels (font atlas pipeline).
- Headless capture producing real PNG + thumbnail.
- Environment enumeration JSON enrichment.

Clarify any ambiguous area before implementing if the spec file doesn’t cover it.
