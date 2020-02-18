##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(10_2_Rendering_Triangle_Colors ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(10_2_Rendering_Triangle_Colors gl3w glfw ${OPENGL_LIBRARIES})
