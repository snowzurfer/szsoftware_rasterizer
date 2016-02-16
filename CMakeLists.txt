
# Cmake entry point
cmake_minimum_required (VERSION 2.8.9)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/rpavlik-cmake-modules-1c73e35/")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} $ENV{PTHREADS-WIN32}/Pre-built.2/lib/x86)
project (szsoftware_rasterizer)

#find_library(PTHREADS_LIB pthreadVC2)
#if(PTHREADS_LIB)
#  message(STATUS ${PTHREADS_LIB})
#endif()

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
  #${TBB_INCLUDE_DIRS}
  #$ENV{PTHREADS-WIN32}/Pre-built.2/include
)

# Set headers and sources for the szutl library
set(SZSOFTRAST_HEADERS
  #${SZSOFTRAST_INCLUDE_DIR}/thread_pool_c.h
  ${SZSOFTRAST_INCLUDE_DIR}/dbg.h
)
set(SZSOFTRAST_SOURCES
  ${SZSOFTRAST_SOURCE_DIR}/main.c
  #${SZSOFTRAST_SOURCE_DIR}/thread_pool_c.c
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

