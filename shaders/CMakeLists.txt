set(GLSL_SOURCES vert.glsl frag.glsl)
compile_glsl(shaders SPV_BINARIES ${GLSL_SOURCES})
add_custom_target(blocco_shaders DEPENDS ${SPV_BINARIES})
