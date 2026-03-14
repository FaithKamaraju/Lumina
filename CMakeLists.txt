cmake_minimum_required(VERSION 4.0)
project(Lumina)

set(CMAKE_CXX_STANDARD 20)


set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

option(LUMINA_BUILD_EDITOR "Build the editor executable" ON)
option(LUMINA_BUILD_GAME_PROJECT "Build sample project targets" ON)
option(LUMINA_SHIPPING "Shipping optimizations on or off" OFF)

if(LUMINA_BUILD_EDITOR AND LUMINA_BUILD_GAME_PROJECT)
    option(LUMINA_HOT_RELOAD "Enable native hot reload module loading" ON)
else()
    option(LUMINA_HOT_RELOAD "Enable native hot reload module loading" OFF)
endif()

add_subdirectory(LuminaEngine)

if(LUMINA_BUILD_EDITOR)
    add_subdirectory(LuminaEditor)
endif()

if(LUMINA_BUILD_GAME_PROJECT)
    add_subdirectory(SampleProject)
endif()


