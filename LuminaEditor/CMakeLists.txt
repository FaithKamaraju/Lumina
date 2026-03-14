cmake_minimum_required(VERSION 4.0)
project(LuminaEditor)

set(CMAKE_CXX_STANDARD 20)

if(MVSC)
    set(CMAKE_C_FLAGS "/EHsc")
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(CMAKE_CXX_FLAGS "/utf-8")
    set(CMAKE_CXX_FLAGS "/utf-8")
endif(MVSC)

if(CMAKE_SYSTEM_NAME EQUAL "Windows")
    set(LE_PLATFORM "LE_PLATFORM_WIN")
endif(CMAKE_SYSTEM_NAME EQUAL "Windows")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LE_PLATFORM "LE_PLATFORM_LINUX")
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

if(CMAKE_SYSTEM_NAME EQUAL "Darwin")
    set(LE_PLATFORM "LE_PLATFORM_MAC")
endif(CMAKE_SYSTEM_NAME EQUAL "Darwin")


# Set the output directory for all runtime targets (executables/DLLs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/$<CONFIG>/bin)

# Optionally, set output directories for static and shared libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/$<CONFIG>/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/$<CONFIG>/lib)


add_executable(${PROJECT_NAME} LuminaEditor.cpp
        src/EditorCamera.cpp
        src/EditorCamera.h
        EditorModule.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ../LuminaEngine/)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${LE_PLATFORM} LE_EDITOR=1)
target_link_libraries(${PROJECT_NAME} PUBLIC LuminaEngine)
