find_package(OpenCV 4.5.4 REQUIRED) #Needs for FaceDetectorYN
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

add_executable(console_app src/main.cpp src/application.cpp)

target_link_libraries(console_app PRIVATE lib_face_detect_api ${JSONCPP_LIBRARIES} ${OpenCV_LIBS})
if(UNIX AND NOT APPLE)
# Link library for dinamic loading of library
    target_link_libraries(console_app PRIVATE dl)
endif()
target_include_directories(console_app PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

#Copy dinamic library in folder with executable
add_custom_command(TARGET console_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:face_detect> $<TARGET_FILE_DIR:console_app>)