#add houdini cmake lists

list( APPEND CMAKE_PREFIX_PATH "C:/Program Files/Side Effects Software/Houdini 17.5.327/toolkit/cmake" )

# Locate Houdini's libraries and header files.
# Registers an imported library target named 'Houdini'.
find_package( Houdini)

file (GLOB_RECURSE Headers "${SOURCE_DIR}/*.h" )
file (GLOB_RECURSE Sources "${SOURCE_DIR}/*.cpp" )

add_library(BubbleH SHARED ${Headers} ${Sources})
target_link_libraries( BubbleH Houdini LosTopos)

houdini_configure_target( BubbleH )