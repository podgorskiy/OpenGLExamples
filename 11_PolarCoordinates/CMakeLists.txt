##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(11_PolarCoordinates ${SOURCES})

##############################################################
# Linkage
##############################################################
target_link_libraries(11_PolarCoordinates gl3w glfw ${OPENGL_LIBRARIES})
