
# Cmake entry point
cmake_minimum_required (VERSION 2.8.9)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/rpavlik-cmake-modules-1c73e35/")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} $ENV{PTHREADS-WIN32}/Pre-built.2/lib/x86)
project (szsoftware_rasterizer)

# Setup handy variables:
set(SZSOFTRAST_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(SZSOFTRAST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SZSOFTRAST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add the threadpool library
set(SZTHREADPOOL_BUILD_EXE FALSE)
add_subdirectory(${SZSOFTRAST_EXTERNAL_DIR}/szthreadpool)

# Set include dirs for the project
include_directories(
  ${SZSOFTRAST_INCLUDE_DIR}
  ${SZTHREADPOOL_INCLUDE_DIR}
)

# Set headers and sources for the szutl library
set(SZSOFTRAST_HEADERS
  ${SZSOFTRAST_INCLUDE_DIR}/dbg.h
  ${SZSOFTRAST_INCLUDE_DIR}/rasterizer.h
)
set(SZSOFTRAST_SOURCES
  ${SZSOFTRAST_SOURCE_DIR}/main.c
  ${SZSOFTRAST_SOURCE_DIR}/rasterizer.c
)

# Create the executable
add_executable(szsoftrast ${SZSOFTRAST_HEADERS} ${SZSOFTRAST_SOURCES})

# Link target (the executable) with libraries
target_link_libraries(szsoftrast
  libszthpool
)

# Set the output directories for the libraries and binaries
set_target_properties (szsoftrast
  PROPERTIES 
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")

