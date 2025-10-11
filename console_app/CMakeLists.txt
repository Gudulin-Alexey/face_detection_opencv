find_package(OpenCV 4.5.4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(console_app src/main.cpp src/application.cpp)

target_link_libraries(console_app PRIVATE lib_face_detect_api ${OpenCV_LIBS})
if(UNIX AND NOT APPLE)
    target_link_libraries(console_app PRIVATE dl)
endif()
target_include_directories(console_app PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)
add_custom_command(TARGET console_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:face_detect> $<TARGET_FILE_DIR:console_app>)