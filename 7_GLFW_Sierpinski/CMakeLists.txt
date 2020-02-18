##############################################################
# Targets
##############################################################
add_executable(7_1_GLFW_basic sources/example1.cpp)
add_executable(7_2_GLFW_glClear sources/example2.cpp)
add_executable(7_3_GLFW_Sierpinski sources/example3.cpp)
add_executable(7_4_GLFW_draw sources/example4.cpp)

##############################################################
# Linkage
##############################################################
target_link_libraries(7_1_GLFW_basic glfw ${OPENGL_LIBRARIES})
target_link_libraries(7_2_GLFW_glClear glfw ${OPENGL_LIBRARIES})
target_link_libraries(7_3_GLFW_Sierpinski glfw ${OPENGL_LIBRARIES})
target_link_libraries(7_4_GLFW_draw glfw ${OPENGL_LIBRARIES})
