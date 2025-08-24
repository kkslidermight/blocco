if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory, e.g. 'cmake -S . -B build'.")
endif()
