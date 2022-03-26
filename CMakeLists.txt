cmake_minimum_required(VERSION 3.17)

project("MantleMap")

set(CMAKE_CXX_STANDARD 17)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h PI_HOST)

# Dependency building

# astro - Make, gets NOVAS-C and JPL Ephemerides
include("cmake/Astro.cmake")

# httplib - Cmake, header-only
add_subdirectory("deps/cpp-httplib")

# fmt - Cmake, header-only
add_subdirectory("deps/fmt")

# json - Cmake, header-only
add_subdirectory("deps/json")

# rpi-rgb-led-matrix, Make, generates native libs

# libpng, webuild this ourselves because oh my god operating systems
add_subdirectory("deps/libpng")

add_executable( ${PROJECT_NAME} 
                    src/Astronomy.cpp
                    src/CmdDebugScene.cpp
                    src/DebugTransformScene.cpp
                    src/GLRenderContext.cpp
                    src/LightScene.cpp
                    src/LoadShaders.cpp
                    src/mantlemap.cpp
                    src/MapState.cpp
                    src/MapTimeScene.cpp
                    src/NaturalEarth.cpp
                    src/PolyLine.cpp
                    src/Scene.cpp
                    src/SolarScene.cpp
                    src/TextLabel.cpp
                    src/WeatherScene.cpp )

# Set some compiler flags used for features
if (PI_HOST)
    target_compile_definitions(${PROJECT_NAME} "PI_HOST")       
    target_compile_definitions(${PROJECT_NAME} "LINUX_HID_CONTROLLER_SUPPORT")
    target_compile_definitions(${PROJECT_NAME} "LED_PANEL_SUPPORT")
endif()

add_dependencies(${PROJECT_NAME} astro)
target_include_directories(${PROJECT_NAME} PRIVATE include ${PNG_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} fmt nlohmann_json httplib png)
