find_package(Vulkan REQUIRED)

function(compile_glsl TARGET OUT_VAR)
  set(SPIRV_OUTPUTS)
  foreach(GLSL ${ARGN})
    get_filename_component(FILE_WE ${GLSL} NAME_WE)
    set(SPV ${CMAKE_BINARY_DIR}/shaders/${FILE_WE}.spv)
    # Infer shader stage from filename (supports vert, frag, comp, geom, rgen, rmiss, rchit minimal set)
    string(TOLOWER "${FILE_WE}" FILE_WE_LC)
    set(STAGE "")
    foreach(candidate IN ITEMS vert frag comp geom rgen rmiss rchit)
      string(FIND "${FILE_WE_LC}" "${candidate}" IDX)
      if(NOT IDX EQUAL -1)
        set(STAGE ${candidate})
        break()
      endif()
    endforeach()
    if(STAGE STREQUAL "")
      message(FATAL_ERROR "Unable to infer shader stage from ${GLSL}; include stage keyword in filename (e.g. name.vert.glsl)")
    endif()
    add_custom_command(OUTPUT ${SPV}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/shaders
      COMMAND ${Vulkan_GLSLC_EXECUTABLE} -fshader-stage=${STAGE} -o ${SPV} ${GLSL}
      DEPENDS ${GLSL}
      COMMENT "Compiling ${GLSL} (${STAGE}) -> ${SPV}")
    list(APPEND SPIRV_OUTPUTS ${SPV})
  endforeach()
  set(${OUT_VAR} ${SPIRV_OUTPUTS} PARENT_SCOPE)
endfunction()
