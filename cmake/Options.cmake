option(BLOCCO_ENABLE_MARCH_NATIVE "Enable -march=native" OFF)
option(BLOCCO_ENABLE_VALIDATION "Enable Vulkan validation layers" ON)
option(BLOCCO_HEADLESS "Build headless capture tool" ON)

if(BLOCCO_ENABLE_MARCH_NATIVE AND NOT MSVC)
  add_compile_options(-march=native)
endif()
