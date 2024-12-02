cmake_minimum_required(VERSION 3.10)
project(mpiLab LANGUAGES CXX)

find_package(MPI REQUIRED)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall")
if(NOT_CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(pa pa.cpp)
add_executable(pb pb.cpp)
add_executable(pc pc.cpp)

get_property(ALL_TARGETS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
foreach(TARGET ${ALL_TARGETS})
    target_link_libraries(${TARGET} PRIVATE MPI::MPI_CXX) 
endforeach()
