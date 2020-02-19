configure_file(albido.pvr albido.pvr COPYONLY)
configure_file(cubemap.pvr cubemap.pvr COPYONLY)
configure_file(normal.pvr normal.pvr COPYONLY)
configure_file(spec.pvr spec.pvr COPYONLY)
configure_file(ibl_brdf.pvr ibl_brdf.pvr COPYONLY)
configure_file(LeePerrySmith.obj LeePerrySmith.obj COPYONLY)

##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES1 20_1_IBL_BRDF_precalc/*.cpp 20_1_IBL_BRDF_precalc/*.h)
file(GLOB_RECURSE SOURCES2 20_2_PBR/*.cpp 20_2_PBR/*.h)
add_executable(20_1_IBL_BRDF_precalc ${SOURCES1})
add_executable(20_2_PBR ${SOURCES2})

##############################################################
# Linkage
##############################################################
target_link_libraries(20_1_IBL_BRDF_precalc ${OPENGL_LIBRARIES})
target_link_libraries(20_2_PBR gl3w glfw imgui  ${OPENGL_LIBRARIES})
