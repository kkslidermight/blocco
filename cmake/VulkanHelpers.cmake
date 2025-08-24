find_package(Vulkan REQUIRED)

function(compile_glsl TARGET OUT_VAR)
  set(SPIRV_OUTPUTS)
  foreach(GLSL ${ARGN})
    get_filename_component(FILE_WE ${GLSL} NAME_WE)
    set(SPV ${CMAKE_CURRENT_BINARY_DIR}/${FILE_WE}.spv)
    add_custom_command(OUTPUT ${SPV}
      COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${SPV} ${GLSL}
      DEPENDS ${GLSL}
      COMMENT "Compiling ${GLSL} -> ${SPV}")
    list(APPEND SPIRV_OUTPUTS ${SPV})
  endforeach()
  set(${OUT_VAR} ${SPIRV_OUTPUTS} PARENT_SCOPE)
endfunction()
