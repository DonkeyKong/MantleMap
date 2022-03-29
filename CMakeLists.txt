cmake_minimum_required(VERSION 3.17)

project("MantleMap")

set(CMAKE_CXX_STANDARD 17)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(/opt/vc/include/bcm_host.h PI_HOST)

# Dependency building
if (APPLE)
set (OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)
endif()
find_package(OpenSSL REQUIRED)

# astro - Fetch and build NOVAS-C and Astro, gets JPL ephemerides
include("cmake/Astro.cmake")

# httplib - Cmake, header-only
add_subdirectory("deps/cpp-httplib")

# fmt - Cmake, header-only
add_subdirectory("deps/fmt")

# json - Cmake, header-only
set(JSON_BuildTests OFF)
add_subdirectory("deps/json")

# rpi-rgb-led-matrix, Make, generates native libs

# libpng, we build this ourselves because oh my god operating systems
if (APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
set(PNG_ARM_NEON on)
endif()
add_subdirectory("deps/libpng")
set(PNG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/libpng
                    ${CMAKE_CURRENT_BINARY_DIR}/deps/libpng)

add_executable( ${PROJECT_NAME} 
                    src/Astronomy.cpp
                    src/CmdDebugScene.cpp
                    src/ConfigCodeScene.cpp
                    src/DebugTransformScene.cpp
                    src/DisplayDevice.cpp
                    src/GLRenderContext.cpp
                    src/ImageRGBA.cpp
                    src/ImageView.cpp
                    src/LightScene.cpp
                    src/LoadShaders.cpp
                    src/mantlemap.cpp
                    src/MapState.cpp
                    src/MapTimeScene.cpp
                    src/NaturalEarth.cpp
                    src/PolyLine.cpp
                    src/Scene.cpp
                    src/SceneElement.cpp
                    src/SolarScene.cpp
                    src/TextLabel.cpp
                    src/UsbButton.cpp
                    src/WeatherScene.cpp
                    deps/QR-Code-Generator/cpp/qrcodegen.cpp )

# Set some compiler flags used for features
if (PI_HOST)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "PI_HOST")       
    target_compile_definitions(${PROJECT_NAME} PUBLIC "LINUX_HID_CONTROLLER_SUPPORT")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "LED_PANEL_SUPPORT")
else()
    include("cmake/Angle.cmake")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "USING_ANGLE")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include ${PNG_INCLUDE_DIR} deps/QR-Code-Generator/cpp)
target_link_libraries(${PROJECT_NAME} fmt nlohmann_json httplib png_static astro angle OpenSSL::SSL OpenSSL::Crypto)

add_custom_target(copy_scenes ALL)

add_custom_command(TARGET copy_scenes POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${PROJECT_SOURCE_DIR}/scenes
                    ${CMAKE_CURRENT_BINARY_DIR}/scenes)
