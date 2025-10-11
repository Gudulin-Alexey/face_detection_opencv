cmake_minimum_required(VERSION 3.14)

project(FaceDetectionPrj CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

add_subdirectory(lib_face_detect)
add_subdirectory(console_app)

