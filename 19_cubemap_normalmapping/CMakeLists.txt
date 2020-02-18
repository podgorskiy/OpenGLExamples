configure_file(albido.tga albido.tga COPYONLY)
configure_file(albido.pvr albido.pvr COPYONLY)
configure_file(cubemap.pvr cubemap.pvr COPYONLY)
configure_file(texture.pvr texture.pvr COPYONLY)
configure_file(normal.pvr normal.pvr COPYONLY)
configure_file(spec.pvr spec.pvr COPYONLY)
configure_file(LeePerrySmith.obj LeePerrySmith.obj COPYONLY)

##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES1 19_1_cubemap/*.cpp src/*.h)
file(GLOB_RECURSE SOURCES2 19_2_sky_box/*.cpp src/*.h)
file(GLOB_RECURSE SOURCES3 19_3_normal_mapping/*.cpp src/*.h)
file(GLOB_RECURSE SOURCES4 19_4_normal_mapping/*.cpp src/*.h)
add_executable(19_1_cubemap ${SOURCES1})
add_executable(19_2_sky_box ${SOURCES2})
add_executable(19_3_normal_mapping ${SOURCES3})
add_executable(19_4_normal_mapping ${SOURCES4})

##############################################################
# Linkage
##############################################################
target_link_libraries(19_1_cubemap gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(19_2_sky_box gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(19_3_normal_mapping gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(19_4_normal_mapping gl3w glfw imgui ${OPENGL_LIBRARIES})
