set(ANGLE_DIR "${PROJECT_SOURCE_DIR}/../angle")

list(APPEND ANGLE_LIBS  angle_deqp_libegl
                        angle_deqp_libgles2
                        #angle_deqp_libgles3_rotate90
                        #angle_deqp_libgles3_rotate180
                        #angle_deqp_libgles3_rotate270
                        angle_deqp_libgles3
                        #angle_deqp_libgles31_rotate90
                        #angle_deqp_libgles31_rotate180
                        #angle_deqp_libgles31_rotate270
                        angle_deqp_libgles31
                        angle_deqp_libkhr_gles2
                        angle_deqp_libkhr_gles3
                        angle_deqp_libkhr_gles31
                        angle_deqp_libkhr_gles32
                        angle_util
                        EGL
                        feature_support
                        GLESv1_CM
                        #GLESv2_with_capture
                        GLESv2
                        metal_shader_cache_file_hooking
                        swiftshader_libEGL
                        swiftshader_libGLESv2
                        vk_swiftshader
                        VkICD_mock_icd )

add_library(angle INTERFACE)
target_include_directories(angle INTERFACE ${ANGLE_DIR})
target_include_directories(angle INTERFACE ${ANGLE_DIR}/include)

foreach(ANGLE_LIB ${ANGLE_LIBS})
    target_link_libraries(angle INTERFACE ${ANGLE_DIR}/out/Release/${CMAKE_SHARED_LIBRARY_PREFIX}${ANGLE_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
endforeach()

add_custom_target(copy_angle ALL)
add_dependencies(copy_angle angle)

foreach(ANGLE_LIB ${ANGLE_LIBS})
    add_custom_command(TARGET copy_angle POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                        ${ANGLE_DIR}/out/Release/${CMAKE_SHARED_LIBRARY_PREFIX}${ANGLE_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX}
                        ${CMAKE_CURRENT_BINARY_DIR}/)
endforeach()
