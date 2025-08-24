add_library(blocco_engine
  engine.hpp engine.cpp
  math.hpp math.cpp
  camera.hpp camera.cpp
  input.hpp input.cpp
  platform.hpp platform.cpp
  renderer.hpp renderer.cpp
  scene.hpp scene.cpp
  labels.hpp labels.cpp
  config.hpp config.cpp
  collision.hpp collision.cpp
  capture.hpp capture.cpp
  logging.hpp logging.cpp
)
set_project_warnings(blocco_engine)
target_link_libraries(blocco_engine PUBLIC SDL3::SDL3 Vulkan::Vulkan)

add_executable(blocco main.cpp)
target_link_libraries(blocco PRIVATE blocco_engine blocco_shaders)

if(BLOCCO_HEADLESS)
  add_executable(blocco_headless headless.cpp)
  target_link_libraries(blocco_headless PRIVATE blocco_engine)
endif()
