set(ANGLE_DIR "${PROJECT_SOURCE_DIR}/../angle")

set(ANGLE_EGL_LIB ${ANGLE_DIR}/out/Release/${CMAKE_SHARED_LIBRARY_PREFIX}EGL${CMAKE_SHARED_LIBRARY_SUFFIX})
set(ANGLE_GLESv2_LIB ${ANGLE_DIR}/out/Release/${CMAKE_SHARED_LIBRARY_PREFIX}GLESv2${CMAKE_SHARED_LIBRARY_SUFFIX})

add_library(angle INTERFACE)
target_link_libraries(angle INTERFACE ${ANGLE_EGL_LIB} ${ANGLE_GLESv2_LIB})
target_include_directories(angle INTERFACE ${ANGLE_DIR}/include)

add_custom_target(copy_angle ALL)
add_dependencies(copy_angle angle)

add_custom_command(TARGET copy_angle POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${ANGLE_EGL_LIB}
                    ${CMAKE_CURRENT_BINARY_DIR}/)

add_custom_command(TARGET copy_angle POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    ${ANGLE_GLESv2_LIB}
                    ${CMAKE_CURRENT_BINARY_DIR}/)