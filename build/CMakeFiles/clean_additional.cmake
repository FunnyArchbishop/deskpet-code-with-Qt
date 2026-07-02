# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\deskpet-qt_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\deskpet-qt_autogen.dir\\ParseCache.txt"
  "deskpet-qt_autogen"
  )
endif()
