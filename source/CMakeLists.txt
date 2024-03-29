#add houdini cmake lists

set(HOUDINI_CMAKE_DIR "C:/Program Files/Side Effects Software/Houdini 18.5.462/toolkit/cmake")

list( APPEND CMAKE_PREFIX_PATH ${HOUDINI_CMAKE_DIR})

# Locate Houdini's libraries and header files.
# Registers an imported library target named 'Houdini'.
find_package( Houdini)

file (GLOB Headers "${SOURCE_DIR}/*.h" )
file (GLOB Sources "${SOURCE_DIR}/*.cpp" )

add_library(BubbleH SHARED ${Headers} ${Sources})
target_link_libraries( BubbleH Houdini LosTopos)

message(STATUS "Houdini directory: ${Houdini_DIR}")

houdini_configure_target( BubbleH )

set_target_properties(BubbleH PROPERTIES VS_GLOBAL_VcpkgEnabled true)

#copy icon to %HOME%/%HOUDINI_VERSION%/config/icons folder

set(TARGET_FILE "${IMG_DIR}/SOP_sop_bubble.svg")
set(TARGET_DIR "")

houdini_get_default_install_dir(TARGET_DIR)

set(TARGET_DIR "${TARGET_DIR}\\config\\icons")

add_custom_command(TARGET BubbleH
					POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy "${TARGET_FILE}" "${TARGET_DIR}"
					COMMENT "Copying ${TARGET_FILE} to ${TARGET_DIR}")

message(STATUS "${TARGET_FILE} will be copied to ${TARGET_DIR}")