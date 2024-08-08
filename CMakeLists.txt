cmake_minimum_required(VERSION 3.20) # Versione minima di CMake
project(audiohook)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include_external_msproject(audiohook ${CMAKE_SOURCE_DIR}/audiohook.vcxproj)