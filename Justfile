set shell := ["fish", "-c"]

# Default recipe lists available tasks
default:
    @echo "Available recipes:"
    @echo "  setup      - Probe/install deps, write env/env.json, init git if needed"
    @echo "  build      - Configure & build (CMake + Ninja)"
    @echo "  run        - Launch blocco executable"
    @echo "  test       - Run unit (and future integration) tests"
    @echo "  diagnose   - Produce diagnostics JSON only"
    @echo "  commit MSG - Build+test gate, run git hook, create commit"

# Probe environment & optionally install missing packages, write env JSON, init git.
setup:
    scripts/env_probe.fish
    if test ! -d .git
        git init
        git add .
        git commit -m 'chore: scaffold init'
    end

# Configure & build
build:
    scripts/build.fish

# Run the main interactive executable
run:
    scripts/run.fish

# Run tests (unit + future integration)
test:
    scripts/test.fish

# Diagnostics only (no install suggestions)
diagnose:
    scripts/env_probe.fish --json-only

# Commit with build/test gating & hook update
# Usage: just commit "feat: message"
commit MESSAGE:
    scripts/git_hook.fish
    scripts/commit.fish "{{MESSAGE}}"
