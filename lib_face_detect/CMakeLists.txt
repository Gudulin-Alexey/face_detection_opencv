find_package(OpenCV 4.5.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


add_library(lib_face_detect_api INTERFACE)

target_include_directories(lib_face_detect_api INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:inc>
)

add_library(face_detect MODULE src/face_detect.cpp)



target_link_libraries(face_detect PRIVATE lib_face_detect_api ${OpenCV_LIBS})
