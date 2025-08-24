# Revised Prompt with References to Similar Projects

## Role & Objective

You are an expert systems engineer and graphics programmer. Generate a complete, auditable C++23 Vulkan + SDL3 3D first-person prototype on Arch Linux under Wayland with KDE Plasma 6. Optimize for reproducibility, explicit configuration, and clear debugging. Use a Justfile for orchestration and Fish scripts where shell specificity is needed.

---

## Deliverables

- A CMake ≥ 3.25 + Ninja repository containing:  
  - **src/**: C++23 source files  
  - **shaders/**: GLSL-to-SPIR-V pipeline (via shaderc/glslc)  
  - **assets/**: minimal fonts/textures (generate at build time)  
  - **cmake/**: utility modules (feature detection, compiler flags, probes)  
  - **tests/**: unit and integration tests  
  - **docs/**: concise README.md with usage and troubleshooting  
  - **.gitignore** tuned for C++/CMake artifacts  
  - **PROGRESS.md** and **TODO.md** (auto-updated by recipes)  
  - **Justfile**: recipes for setup, build, run, test, diagnose, and commit  
  - **scripts/env_probe.fish**: Fish probe for packages and env/env.json  
  - **scripts/git_hook.fish**: Fish hook for conventional commits

- Git initialized by the `just setup` recipe and committed via the `just commit` recipe.

---

## Hard Requirements

### Language & Tooling

- C++23 only (no engines or scene-graphs), Vulkan C API or vulkan-hpp, SDL3.  
- CMake ≥ 3.25 with Ninja generator. Provide `Debug` and `RelWithDebInfo` presets.  
- Compile shaders to SPIR-V at build time; enable validation layers in Debug.

### Platform & Runtime

- Prefer Wayland via SDL3; detect X11 at runtime with graceful fallback.  
- Implement pointer lock and relative mouse for mouselook under Wayland.

### Gameplay & Scene

- First-person camera: relative mouselook with clamped pitch and configurable sensitivity.  
- WASD movement, Space to jump, gravity, and axis-aligned bounding box collision.  
- Spawn in a sealed rectangular room (floor, ceiling, and four walls), each with a distinct color and large, legible label: “FLOOR”, “CEILING”, “FRONT WALL”, “BACK WALL”, “LEFT WALL”, “RIGHT WALL”.  
- Static “character model” composed of six axis-aligned cubes (legs, arms, body, head) visible from first-person view.

### Rendering Pipeline

- Vulkan swapchain, depth buffer, and basic Blinn-Phong lighting.  
- Use vertex/index buffers, uniform buffers, descriptor sets, render pass, and pipeline cache.  
- Robust synchronization with fences/semaphores and proper resize handling.

### Text & Labels

- Either a build-time bitmap font atlas textured onto surfaces or a screen-space overlay anchored per surface.  
- Ensure high-contrast, legible labels.

### Controls & Config

- ESC: quit  
- F1: toggle debug HUD (FPS, frame time, camera pos, GPU info)  
- F2: toggle wireframe  
- F12: capture frame dump (PNG + thumbnail)  
- Config file (TOML) for sensitivity, keybinds, colors, window size, vsync, validation toggles

---

## Task Orchestration with Justfile

Use a `Justfile` to unify your workflows across shells:

- **setup**:  
  - Invoke `fish scripts/env_probe.fish` to install missing packages and write `env/env.json`.  
  - Initialize Git if needed and create the “chore: initial project scaffold” commit.  

- **build**:  
  - Configure and build the project with CMake + Ninja.  

- **run**:  
  - Launch the executable with the default config.  

- **test**:  
  - Run unit and integration tests.  

- **diagnose**:  
  - Invoke `fish scripts/env_probe.fish --json-only` for diagnostics.  

- **commit** MESSAGE:  
  - Run build and test.  
  - Invoke `fish scripts/git_hook.fish` to update PROGRESS.md/TODO.md and enforce conventional commit messages.  
  - Stage and commit with the provided MESSAGE.

Reserve Fish scripts for shell-specific tasks:

- **scripts/env_probe.fish**: detect `pacman -Qi` packages, drivers, and write `env/env.json`.  
- **scripts/git_hook.fish**: enforce commit message conventions, update PROGRESS.md and TODO.md.

---

## Environment Setup & Auditability

- **env_probe.fish** must:  
  - Probe base-devel, cmake, ninja, gcc/clang, git, sdl3, Vulkan libs/tools, shaderc, spirv-tools, Mesa/vendor drivers, Wayland, libxkbcommon.  
  - Write `env/env.json` capturing GPU/vendor, driver versions, Vulkan API/features, CPU model/flags, OS/kernel, compositor, SDL backend, and `pacman -Qi` package versions.

- CMake must support:  
  - `-march=native` off by default, with fallback flags via try-compile.  
  - Optional features gated by flags with deterministic defaults.

---

## Debugging & Visibility

- F12: save current swapchain image + thumbnail into `debug/captures/YYYYMMDD_HHMMSS.png`.  
- On crash or validation error: capture the last frame if possible.  
- Headless-capture mode: `just run -- --headless-capture=N` renders N frames offscreen and saves PNGs.  
- Log camera position, orientation, FPS, GPU details, and pipeline state into `debug/session.log`.

---

## Testing & CI

1. Unit tests for vector/matrix math, AABB collision, and camera transforms.  
2. Integration tests:  
   - Launch and render 60 frames; verify no validation errors and non-empty buffer PNG.  
   - Simulate movement input and confirm camera displacement and collision responses.  
3. Enable validation layers by default in Debug; filter and log messages.  
4. Provide a reproducible crash path behind a flag to test error handling.

---

## Examples of Similar Projects for Reference

- **HelloVulkanSDL**: A minimal Vulkan + SDL3 skeleton in C using CMake and shader compilation scripts.  
- **c_vulkan_sdl3**: Plain C example rendering a rotating colored square in an SDL3 window, with build scripts and GLSL shaders.  
- **Vulkaning**: A collection of experimental scratch-built Vulkan hpp + SDL3 projects demonstrating various rendering techniques.  
- **bgfx**: Cross-platform, graphics API-agnostic rendering library offering insights into pipeline abstraction (C++, supports Vulkan).  

---

## Acceptance Criteria

- `just setup` installs dependencies, probes environment, initializes Git, and commits scaffold.  
- `just build`, `just run`, `just test`, `just diagnose`, and `just commit "MSG"` execute without manual fixes.  
- Application launches into a brightly lit 3D room with correctly colored and labeled surfaces.  
- Mouse look, WASD movement, jump, gravity, and collision behave as specified.  
- F1 toggles the HUD; F12 captures PNG + thumbnail; ESC cleanly exits.  
- PROGRESS.md and TODO.md accurately reflect progress; Git history shows logical, small commits.  
- `env/env.json` is produced; debug logs and captures are present; headless capture works.

---

Now produce the full repository with source code, CMake configuration, Justfile, Fish scripts, tests, shaders, and documentation exactly as specified. Ensure all configs are explicit, reproducible, and audit-friendly.