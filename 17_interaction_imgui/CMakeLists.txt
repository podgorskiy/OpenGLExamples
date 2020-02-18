##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES 17_1_imgui/sources/*.cpp src/*.h)
add_executable(17_1_imgui ${SOURCES})
file(GLOB_RECURSE SOURCES 17_2_imgui/sources/*.cpp src/*.h)
add_executable(17_2_imgui ${SOURCES})
file(GLOB_RECURSE SOURCES 17_3_window_resize/sources/*.cpp src/*.h)
add_executable(17_3_window_resize ${SOURCES})
file(GLOB_RECURSE SOURCES 17_4_window_resize/sources/*.cpp src/*.h)
add_executable(17_4_window_resize ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(17_1_imgui gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(17_2_imgui gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(17_3_window_resize gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(17_4_window_resize gl3w glfw imgui ${OPENGL_LIBRARIES})
