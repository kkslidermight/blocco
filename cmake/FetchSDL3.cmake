include(FetchContent)

# Use an archive instead of a git clone to avoid embedding a nested .git repo
# (reduces IDE noise about thousands of pending changes and improves reproducibility).
set(SDL3_REF main)
FetchContent_Declare(SDL3
	URL https://github.com/libsdl-org/SDL/archive/refs/heads/${SDL3_REF}.tar.gz
	# TODO: Pin to a specific commit tarball for reproducibility e.g.
	# URL https://github.com/libsdl-org/SDL/archive/<commit>.tar.gz
	# URL_HASH SHA256=<insert_hash>
)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)
