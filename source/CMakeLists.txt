#add houdini cmake lists

set(HOUDINI_CMAKE_DIR "C:/Program Files/Side Effects Software/Houdini 17.5.327/toolkit/cmake")

list( APPEND CMAKE_PREFIX_PATH ${HOUDINI_CMAKE_DIR})

# Locate Houdini's libraries and header files.
# Registers an imported library target named 'Houdini'.
find_package( Houdini)

file (GLOB_RECURSE Headers "${SOURCE_DIR}/*.h" )
file (GLOB_RECURSE Sources "${SOURCE_DIR}/*.cpp" )

add_library(BubbleH SHARED ${Headers} ${Sources})
target_link_libraries( BubbleH Houdini LosTopos)

message(STATUS "Houdini directory: ${Houdini_DIR}")

houdini_configure_target( BubbleH )