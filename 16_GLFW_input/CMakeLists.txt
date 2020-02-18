##############################################################
# Targets
##############################################################
add_executable(16_1_GLFW_input sources/example1.cpp)
add_executable(16_2_GLFW_input sources/example2.cpp)
add_executable(16_3_GLFW_input sources/example3.cpp)

##############################################################
# Linkage
##############################################################
target_link_libraries(16_1_GLFW_input glfw ${OPENGL_LIBRARIES})
target_link_libraries(16_2_GLFW_input glfw ${OPENGL_LIBRARIES})
target_link_libraries(16_3_GLFW_input glfw ${OPENGL_LIBRARIES})
