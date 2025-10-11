find_package(OpenCV 4.5.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_library(lib_face_detect MODULE src/face_detect.cpp)

target_include_directories(lib_face_detect PUBLIC
    $<BUILD_INTERFACE:${CMAKE_INCLUDE_CURRENT_DIR}/inc>
    $<INSTALL_INTERFACE:inc>
)

target_link_libraries(lib_face_detect PRIVATE ${OpenCV_LIBS})
