##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(10_3_Uniforms ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(10_3_Uniforms gl3w glfw ${OPENGL_LIBRARIES})
