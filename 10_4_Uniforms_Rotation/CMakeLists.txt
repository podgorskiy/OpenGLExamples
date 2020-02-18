##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(10_4_Uniforms_Rotation ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(10_4_Uniforms_Rotation gl3w glfw ${OPENGL_LIBRARIES})
