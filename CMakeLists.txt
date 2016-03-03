
# Cmake entry point
cmake_minimum_required (VERSION 2.8.9)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/rpavlik-cmake-modules-1c73e35/")

project (szrasterizer)

# Setup handy variables:
set(SZRAST_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SZRAST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SZRAST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add the threadpool library
set(SZTHREADPOOL_BUILD_EXE FALSE)
add_subdirectory(${SZRAST_EXTERNAL_DIR}/szthreadpool)

# Add the kazmath library
option(KAZMATH_BUILD_TESTS OFF)
option(KAZMATH_BUILD_GL_UTILS OFF)
option(KAZMATH_BUILD_JNI_WRAPPER OFF)
add_subdirectory(${SZRAST_EXTERNAL_DIR}/kazmath)

# Set include dirs for the project
include_directories(
  ${SZRAST_INCLUDE_DIR}
  ${SZTHREADPOOL_INCLUDE_DIR}
  ${SZRAST_EXTERNAL_DIR}/kazmath/kazmath
)

# Set headers and sources for the szutl library
set(SZRAST_HEADERS
  ${SZRAST_INCLUDE_DIR}/dbg.h
  ${SZRAST_INCLUDE_DIR}/rasterizer.h
  ${SZRAST_SOURCE_DIR}/vertex_buffer.h
  ${SZRAST_SOURCE_DIR}/index_buffer.h
  ${SZRAST_SOURCE_DIR}/render_target.h
  ${SZRAST_SOURCE_DIR}/command_buffer.h
  ${SZRAST_SOURCE_DIR}/device.h
)
set(SZRAST_SOURCES
  ${SZRAST_SOURCE_DIR}/main.c
  ${SZRAST_SOURCE_DIR}/rasterizer.c
  ${SZRAST_SOURCE_DIR}/vertex_buffer.c
  ${SZRAST_SOURCE_DIR}/index_buffer.c
  ${SZRAST_SOURCE_DIR}/render_target.c
  ${SZRAST_SOURCE_DIR}/command_buffer.c
  ${SZRAST_SOURCE_DIR}/device.c
)

# Create the executable
add_executable(szrast ${SZRAST_HEADERS} ${SZRAST_SOURCES})

# Link target (the executable) with libraries
find_library(M_LIB m)
target_link_libraries(szrast
  libszthpool
  kazmath
  ${M_LIB}
)

# Set the output directories for the libraries and binaries
set_target_properties (szrast
  PROPERTIES 
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")

