##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(9_2_Triangle_VertexBuffers_CPU ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(9_2_Triangle_VertexBuffers_CPU gl3w glfw ${OPENGL_LIBRARIES})
