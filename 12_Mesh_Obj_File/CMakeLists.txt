configure_file(albido.tga albido.tga COPYONLY)
configure_file(LeePerrySmith.obj LeePerrySmith.obj COPYONLY)

##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES1 12_1_Mesh/*.*)
add_executable(12_1_Mesh ${SOURCES1})
file(GLOB_RECURSE SOURCES2 12_2_Mesh_UV_Painted/*.*)
add_executable(12_2_Mesh_UV_Painted ${SOURCES2})
file(GLOB_RECURSE SOURCES3 12_3_MeshTextured/*.*)
add_executable(12_3_MeshTextured ${SOURCES3})
file(GLOB_RECURSE SOURCES4 12_4_MeshTextured_and_Z_Buffer/*.*)
add_executable(12_4_MeshTextured_and_Z_Buffer ${SOURCES4})

##############################################################
# Linkage
##############################################################
target_link_libraries(12_1_Mesh gl3w glfw ${OPENGL_LIBRARIES})
target_link_libraries(12_2_Mesh_UV_Painted gl3w glfw ${OPENGL_LIBRARIES})
target_link_libraries(12_3_MeshTextured gl3w glfw ${OPENGL_LIBRARIES})
target_link_libraries(12_4_MeshTextured_and_Z_Buffer gl3w glfw ${OPENGL_LIBRARIES})
