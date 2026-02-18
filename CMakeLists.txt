cmake_minimum_required(VERSION 4.0)
project(Lumina)

set(CMAKE_CXX_STANDARD 20)

# Set the output directory for all runtime targets (executables/DLLs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject/Binaries/$<CONFIG>/bin)

# Optionally, set output directories for static and shared libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject/Binaries/$<CONFIG>/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/SampleProject/Binaries/$<CONFIG>/lib)


set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_subdirectory(LuminaEngine)
add_subdirectory(SampleProject)


