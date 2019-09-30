add_definitions(-DUSE_FORTRAN_BLAS)

file (GLOB_RECURSE Headers "${LOS_TOPOS_DIR}/LosTopos3D/*.h" )
file (GLOB_RECURSE Headers "${LOS_TOPOS_DIR}/common/*.h" )

file (GLOB_RECURSE Sources "${LOS_TOPOS_DIR}/LosTopos3D/*.cpp" )
file (GLOB_RECURSE Sources "${LOS_TOPOS_DIR}/common/*.cpp" )

find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIR})

find_package (GLUT REQUIRED glut)
include_directories (${GLUT_INCLUDE_DIR})

add_library(LosTopos STATIC ${Headers} ${Sources})
target_link_libraries (LosTopos ${DEFAULT_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_glut_LIBRARY})


