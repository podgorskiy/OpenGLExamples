# configure_file(albido.tga albido.tga COPYONLY)
configure_file(LeePerrySmith.obj LeePerrySmith.obj COPYONLY)

##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES1 18_1_vertex_shading/*.cpp 18_1_vertex_shading/*.h)
add_executable(18_1_vertex_shading ${SOURCES1})
file(GLOB_RECURSE SOURCES2 18_2_fragment_shading/*.cpp 18_2_fragment_shading/*.h)
add_executable(18_2_fragment_shading ${SOURCES2})
file(GLOB_RECURSE SOURCES3 18_3_face_culling/*.cpp 18_3_face_culling/*.h)
add_executable(18_3_face_culling ${SOURCES3})
file(GLOB_RECURSE SOURCES4 18_4_sky_illumination/*.cpp 18_4_sky_illumination/*.h)
add_executable(18_4_sky_illumination ${SOURCES4})

##############################################################
# Linkage
##############################################################
target_link_libraries(18_1_vertex_shading gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(18_2_fragment_shading gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(18_3_face_culling gl3w glfw imgui ${OPENGL_LIBRARIES})
target_link_libraries(18_4_sky_illumination gl3w glfw imgui ${OPENGL_LIBRARIES})
