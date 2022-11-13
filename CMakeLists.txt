cmake_minimum_required(VERSION 3.10)
project(StopWait)

# People keep running CMake in the wrong folder, completely nuking their project or creating weird bugs.
# This checks if you're running CMake from a folder that already has CMakeLists.txt.
# Importantly, this catches the common case of running it from the root directory.
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)
if (EXISTS "${PATH_TO_CMAKELISTS_TXT}")
    message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to `Debug` as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(CMAKE_C_COMPTLER GCC)
set(CMAKE_CXX FLAGS -fexec-charset=GBK)
add_definitions(-std=c++11)

include_directories(${PROJECT_SOURCE_DIR}/include)
find_library(NETSIM_LIB libnetsim.a ${PROJECT_SOURCE_DIR}/lib)

# SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/stopwait SRC_LIST)
add_executable(stop_wait ${SRC_LIST})
target_link_libraries(stop_wait  ${NETSIM_LIB})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/gbn SRC_LIST_GBN)
add_executable(gbn ${SRC_LIST_GBN})
target_link_libraries(gbn ${NETSIM_LIB})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/sr SRC_LIST_SR)
add_executable(sr ${SRC_LIST_SR})
target_link_libraries(sr ${NETSIM_LIB})

aux_source_directory(${PROJECT_SOURCE_DIR}/src/tcp SRC_LIST_TCP)
add_executable(tcp ${SRC_LIST_TCP})
target_link_libraries(tcp ${NETSIM_LIB})