cmake_minimum_required(VERSION 4.0)
project(LuminaEngine C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 17)

if(MVSC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHsc")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/EHsc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif(MVSC)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(LE_PLATFORM "LE_PLATFORM_WIN")
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LE_PLATFORM "LE_PLATFORM_LINUX")
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(LE_PLATFORM "LE_PLATFORM_MAC")
endif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

set(SHADERC_ENABLE_SHARED_CRT ON)
#set(IMGUI_DIR external/imgui)
#set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_EXAMPLES ON)


find_package(Vulkan REQUIRED)
add_subdirectory(external/glfw/)
add_subdirectory(external/glm/)
add_subdirectory(external/spdlog/)
add_subdirectory(external/shaderc/)
add_subdirectory(external/fastgltf/)
add_subdirectory(external/meshopt/)
#add_subdirectory(external/volk/)

add_library(VulkanCppModule INTERFACE)
add_library(Vulkan::cppm ALIAS VulkanCppModule)
target_link_libraries(VulkanCppModule INTERFACE Vulkan::Vulkan)
target_compile_definitions(VulkanCppModule
        INTERFACE   VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
        VULKAN_HPP_NO_SPACESHIP_OPERATOR=1
        VK_ENABLE_BETA_EXTENSIONS=1
        XXH_NO_EXTERNC_GUARD=1
)

# Set the output directory for all runtime targets (executables/DLLs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Binaries/$<CONFIG>/bin)

# Optionally, set output directories for static and shared libraries
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Binaries/$<CONFIG>/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Binaries/$<CONFIG>/lib)

file(GLOB_RECURSE LUMINA_ENGINE_SOURCES
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

list(FILTER LUMINA_ENGINE_SOURCES EXCLUDE REGEX ".*external/.*")

add_library(LuminaEngine STATIC

        #xxHash files
        external/xxHash/xxhash.c

        # enkiTS files
        external/enkiTS/src/TaskScheduler.cpp

        # mINI files
        external/mINI/src/mini/ini.h

        # stb_image files
        external/stbimage/stb_image.cpp

        # Vulkan Memory Allocator
        external/vma/src/VmaUsage.h
        external/vma/src/VmaUsage.cpp

        ${LUMINA_ENGINE_SOURCES})


set_source_files_properties(external/xxHash/xxhash.c PROPERTIES LANGUAGE CXX)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm spdlog Vulkan::cppm shaderc_combined fastgltf meshoptimizer )

target_precompile_headers(${PROJECT_NAME} PRIVATE lepch.h)



set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${LE_PLATFORM})
if (LUMINA_HOT_RELOAD)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LE_HOT_RELOAD=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE LE_HOT_RELOAD=0)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC external/glfw/include/)
target_include_directories(${PROJECT_NAME} PUBLIC external/glm/)
target_include_directories(${PROJECT_NAME} PUBLIC external/spdlog/include/)
target_include_directories(${PROJECT_NAME} PUBLIC external/mINI/src/)
target_include_directories(${PROJECT_NAME} PUBLIC external/stbImage/)
target_include_directories(${PROJECT_NAME} PRIVATE external/enkiTS/src/)
target_include_directories(${PROJECT_NAME} PRIVATE external/xxHash/)
target_include_directories(${PROJECT_NAME} PUBLIC external/cereal/include/)
target_include_directories(${PROJECT_NAME} PRIVATE external/vma/include/)
target_include_directories(${PROJECT_NAME} PRIVATE external/vma/src/)
target_include_directories(${PROJECT_NAME} PRIVATE external/shaderc/libshaderc/include/)
target_include_directories(${PROJECT_NAME} PRIVATE external/shaderc/libshaderc_util/include/)
target_include_directories(${PROJECT_NAME} PRIVATE external/fastgltf/include)

target_include_directories(${PROJECT_NAME} PRIVATE ./)

