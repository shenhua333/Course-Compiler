# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\CompilerFrontend2_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CompilerFrontend2_autogen.dir\\ParseCache.txt"
  "CompilerFrontend2_autogen"
  )
endif()
