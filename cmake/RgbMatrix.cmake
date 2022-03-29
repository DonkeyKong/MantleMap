include(ExternalProject)

find_program(MAKE_EXECUTABLE NAMES gmake make mingw32-make REQUIRED)

set(rgbmatrix_LIBRARY ${PROJECT_SOURCE_DIR}/deps/rpi-rgb-led-matrix/lib/${CMAKE_STATIC_LIBRARY_PREFIX}rgbmatrix${CMAKE_STATIC_LIBRARY_SUFFIX})
set(rgbmatrix_INCLUDE ${PROJECT_SOURCE_DIR}/deps/rpi-rgb-led-matrix/include)

ExternalProject_Add(rgbmatrix_build
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/rpi-rgb-led-matrix
    CONFIGURE_COMMAND ""
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/rpi-rgb-led-matrix
    BUILD_COMMAND ${MAKE_EXECUTABLE} -C ${PROJECT_SOURCE_DIR}/deps/rpi-rgb-led-matrix
    INSTALL_COMMAND ""
)

add_library(rgbmatrix INTERFACE)
target_link_libraries(rgbmatrix INTERFACE ${rgbmatrix_LIBRARY})
target_include_directories(rgbmatrix INTERFACE ${rgbmatrix_INCLUDE})
add_dependencies(rgbmatrix rgbmatrix_build)
