find_package(GTest REQUIRED)

add_executable(tests test_lib.cpp)
target_link_libraries(tests PRIVATE GTest::gtest_main face_detect lib_face_detect_api)

include(GoogleTest)
gtest_discover_tests(tests)