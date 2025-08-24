#include "platform.hpp"
#include <cstdlib>
namespace Platform { std::string compositor(){ const char* w = std::getenv("XDG_SESSION_TYPE"); return w? w: "unknown"; }}
