
##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES1 20_1_IBL_BRDF_precalc/*.cpp src/*.h)
add_executable(20_1_IBL_BRDF_precalc ${SOURCES1})

##############################################################
# Linkage
##############################################################
target_link_libraries(20_1_IBL_BRDF_precalc ${OPENGL_LIBRARIES})
