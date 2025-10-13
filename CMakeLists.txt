cmake_minimum_required(VERSION 3.14)

project(FaceDetectionPrj CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(lib_face_detect)
add_subdirectory(console_app)
add_subdirectory(tests)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

