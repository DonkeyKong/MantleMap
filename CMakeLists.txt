cmake_minimum_required(VERSION 3.16)

project("MantleMap")

set(CMAKE_CXX_STANDARD 17)

find_file(BCM_HOST_PATH NAMES "bcm_host.h" PATHS /opt/vc/include/)
message("BCM host header: ${BCM_HOST_PATH}")

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
if (BCM_HOST_PATH)
    include("cmake/RgbMatrix.cmake")
endif()

# libpng, we build this ourselves because oh my god operating systems
if (APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
set(PNG_ARM_NEON on)
endif()
set(PNG_SHARED OFF)
set(PNG_TESTS OFF)
add_subdirectory("deps/libpng")
set(PNG_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/deps/libpng
                    ${CMAKE_CURRENT_BINARY_DIR}/deps/libpng)

add_executable( ${PROJECT_NAME} 
                    src/Attributes.cpp
                    src/AstronomyService.cpp
                    src/CmdDebugScene.cpp
                    src/ConfigCodeScene.cpp
                    src/DebugTransformScene.cpp
                    src/DisplayDevice.cpp
                    src/GLRenderContext.cpp
                    src/HttpService.cpp
                    src/ImageRGBA.cpp
                    src/ImageView.cpp
                    src/LightScene.cpp
                    src/LoadShaders.cpp
                    src/main.cpp
                    src/ConfigService.cpp
                    src/MapTimeScene.cpp
                    src/NaturalEarth.cpp
                    src/PhysicsScene.cpp
                    src/PolyLine.cpp
                    src/Scene.cpp
                    src/SceneElement.cpp
                    src/SolarScene.cpp
                    src/TextLabel.cpp
                    src/TimeService.cpp
                    src/InputButton.cpp
                    src/Utils.cpp
                    src/WeatherScene.cpp
                    deps/QR-Code-generator/cpp/qrcodegen.cpp )

# Set some compiler flags used for features
if (BCM_HOST_PATH)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "PI_HOST")       
    target_compile_definitions(${PROJECT_NAME} PUBLIC "NPOT_TEXTURE_SUPPORT")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "LINUX_HID_CONTROLLER_SUPPORT")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "LED_PANEL_SUPPORT")
    target_link_libraries(${PROJECT_NAME} rgbmatrix)
    target_include_directories(${PROJECT_NAME} PRIVATE "/opt/vc/include")
    target_link_directories(${PROJECT_NAME} PRIVATE "/opt/vc/lib")
    target_link_libraries(${PROJECT_NAME} brcmGLESv2 brcmEGL bcm_host)
else()
    include("cmake/Angle.cmake")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "NPOT_TEXTURE_SUPPORT")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "USING_ANGLE")
    target_link_libraries(${PROJECT_NAME} angle)
endif()

# Link the c++ filesystem API under GCC
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE include ${PNG_INCLUDE_DIR} deps/QR-Code-generator/cpp)
target_link_libraries(${PROJECT_NAME} fmt nlohmann_json httplib png_static astro OpenSSL::SSL OpenSSL::Crypto)

add_custom_target(copy_scenes ALL)

add_custom_command(TARGET copy_scenes POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${PROJECT_SOURCE_DIR}/scenes
                    ${CMAKE_CURRENT_BINARY_DIR}/scenes)
