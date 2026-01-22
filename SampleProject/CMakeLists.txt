cmake_minimum_required(VERSION 4.0)
project(SampleProject)

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

if(CMAKE_SYSTEM_NAME EQUAL "Darwin")
    set(LE_PLATFORM "LE_PLATFORM_MAC")
endif(CMAKE_SYSTEM_NAME EQUAL "Darwin")


add_executable(${PROJECT_NAME} SampleGame.cpp
        SampleGame.h)

target_include_directories(${PROJECT_NAME} PRIVATE ../LuminaEngine/)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${LE_PLATFORM})
target_link_libraries(${PROJECT_NAME} PRIVATE LuminaEngine)
