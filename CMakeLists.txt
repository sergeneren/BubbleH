cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)

project (BubbleH)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions (-DDEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Debug)


get_filename_component(ELTOPO_DIR ${CMAKE_SOURCE_DIR}/thirdparty/eltopo ABSOLUTE)

# Find required packages 
find_package (Eigen3 REQUIRED)
find_package (CLAPACK REQUIRED)
find_package (BLAS REQUIRED)





include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eltopo/eltopo3D)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eltopo/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eltopo/common/newsparse)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/eltopo/common/tunicate)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Apps/SoapFilm3D/fmmtl)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory (thirdparty/eltopo)