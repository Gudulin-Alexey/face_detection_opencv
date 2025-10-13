#include "gtest/gtest.h"
#include "face_detect_api.h"

TEST(FaceDetectionTest, EmptyFileName) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    EXPECT_EQ(detect("", buf,bufsize),-1);
}