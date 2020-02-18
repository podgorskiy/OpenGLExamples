##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(10_1_Triangle_VertexBuffers_GPU ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(10_1_Triangle_VertexBuffers_GPU gl3w glfw ${OPENGL_LIBRARIES})
