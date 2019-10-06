cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project (BubbleH LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions (-DDEBUG)
  string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(CONCAT CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "/Z7")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

get_filename_component(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source ABSOLUTE)
get_filename_component(IMG_DIR ${CMAKE_SOURCE_DIR}/img ABSOLUTE)
get_filename_component(LOS_TOPOS_DIR ${CMAKE_SOURCE_DIR}/thirdparty/LosTopos ABSOLUTE)

# Find required packages 
find_package (Eigen3 REQUIRED)
find_package (CLAPACK REQUIRED)
find_package (BLAS REQUIRED)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LosTopos/LosTopos3D)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LosTopos/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LosTopos/common/tunicate)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/LosTopos/common/newsparse)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory (source)
add_subdirectory (thirdparty/LosTopos)