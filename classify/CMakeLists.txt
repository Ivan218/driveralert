cmake_minimum_required(VERSION 3.6)
project(RTclassify)

set(CMAKE_CXX_STANDARD 14)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS})

set(SOURCE_FILES run_classifier.cpp VideoFaceDetector.h VideoFaceDetector.cpp)
add_executable(classify ${SOURCE_FILES})

target_link_libraries(classify ${OpenCV_LIBS})