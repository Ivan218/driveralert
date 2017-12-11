cmake_minimum_required(VERSION 3.6)
project(personaldetection)

set (CMAKE_CXX_STANDARD 14)

#OpenCV
find_package(OpenCV REQUIRED)
#include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCE_FILES sift.cpp)
add_executable(sift ${SOURCE_FILES})

target_link_libraries(sift ${OpenCV_LIBS})
