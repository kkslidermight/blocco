set(GLSL_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/vert.glsl
	${CMAKE_CURRENT_SOURCE_DIR}/frag.glsl
)
compile_glsl(shaders SPV_BINARIES ${GLSL_SOURCES})
add_custom_target(blocco_shaders DEPENDS ${SPV_BINARIES})
