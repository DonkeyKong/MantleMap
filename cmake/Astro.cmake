include(ExternalProject)

find_program(MAKE_EXECUTABLE NAMES gmake make mingw32-make REQUIRED)

set(NOVAS_CDIST novasc3.1)
set(ephutil_LIBRARY ${PROJECT_SOURCE_DIR}/deps/astro/ephutil/ephutil${CMAKE_STATIC_LIBRARY_SUFFIX})
set(novas_LIBRARY ${PROJECT_SOURCE_DIR}/deps/astro/${NOVAS_CDIST}/novas${CMAKE_STATIC_LIBRARY_SUFFIX})
set(novas_EPHEMERIDES ${PROJECT_SOURCE_DIR}/deps/astro/linux_p1550p2650.430)

ExternalProject_Add(astro_build
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/deps/astro
    CONFIGURE_COMMAND ""
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/deps/astro
    BUILD_COMMAND ${MAKE_EXECUTABLE} -j -C ${PROJECT_SOURCE_DIR}/deps/astro DE=430
    INSTALL_COMMAND mkdir -p ${CMAKE_BINARY_DIR}/deps/astro/lib &&
                    cp ${novas_LIBRARY} ${CMAKE_BINARY_DIR}/deps/astro/lib/ &&
                    cp ${ephutil_LIBRARY} ${CMAKE_BINARY_DIR}/deps/astro/lib/ &&
                    mkdir -p ${CMAKE_BINARY_DIR}/deps/astro/include &&
                    cp ${PROJECT_SOURCE_DIR}/deps/astro/${NOVAS_CDIST}/novas.h ${CMAKE_BINARY_DIR}/deps/astro/include/ &&
                    cp ${PROJECT_SOURCE_DIR}/deps/astro/${NOVAS_CDIST}/novascon.h ${CMAKE_BINARY_DIR}/deps/astro/include/ &&
                    cp ${PROJECT_SOURCE_DIR}/deps/astro/${NOVAS_CDIST}/solarsystem.h ${CMAKE_BINARY_DIR}/deps/astro/include/ &&
                    cp ${PROJECT_SOURCE_DIR}/deps/astro/${NOVAS_CDIST}/nutation.h ${CMAKE_BINARY_DIR}/deps/astro/include/ &&
                    cp ${PROJECT_SOURCE_DIR}/deps/astro/${NOVAS_CDIST}/eph_manager.h ${CMAKE_BINARY_DIR}/deps/astro/include/ &&
                    cp ${PROJECT_SOURCE_DIR}/deps/astro/ephutil/ephutil.h ${CMAKE_BINARY_DIR}/deps/astro/include/
                    
    BUILD_BYPRODUCTS ${ephutil_LIBRARY} ${novas_LIBRARY} ${novas_EPHEMERIDES}
)

add_library(astro INTERFACE)
target_link_libraries(astro INTERFACE ${ephutil_LIBRARY} ${novas_LIBRARY})
target_include_directories(astro INTERFACE ${CMAKE_BINARY_DIR}/deps/astro/include)
add_dependencies(astro astro_build)

add_custom_target(copy_ephemerides ALL)
add_dependencies(copy_ephemerides astro)
add_custom_command(TARGET copy_ephemerides POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${PROJECT_SOURCE_DIR}/deps/astro/linux_p1550p2650.430 
                    ${CMAKE_CURRENT_BINARY_DIR}/)
