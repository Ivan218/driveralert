cmake_minimum_required(VERSION 3.6)
project(faceDetection)

set(CMAKE_CXX_STANDARD 14)

#file(COPY haarcascade_frontalface_default.xml DESTINATION ${PROJECT_BINARY_DIR})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCE_FILES newdetect.cpp) 
add_executable(newdetect ${SOURCE_FILES})

target_link_libraries(newdetect ${OpenCV_LIBS})
