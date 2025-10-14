#include "gtest/gtest.h"
#include "face_detect_api.h"

TEST(FaceDetectionTest, EmptyFileName) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    EXPECT_EQ(detect("", buf,bufsize),-1);
}

TEST(FaceDetectionTest, SingleFace) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    EXPECT_EQ(detect("single_face.jpg", buf,bufsize),1);
    auto& rect = buf[0];

    EXPECT_TRUE((rect.x >= 340) && (rect.x <= 380));
    EXPECT_TRUE((rect.y >= 85) && (rect.y <= 120));
    EXPECT_TRUE((rect.w >= 120) && (rect.w <= 160));
    EXPECT_TRUE((rect.h >= 145) && (rect.h <= 185));
}

TEST(FaceDetectionTest, MultiFace) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    int count = detect("many_faces.jpg", buf,bufsize);
    EXPECT_GT(count,5);
    for (int i = 0; i < count; i++) {
        auto& rect = buf[i];
        EXPECT_GE(rect.x, 0);
        EXPECT_GE(rect.y, 0);
        EXPECT_GT(rect.w, 10);
        EXPECT_GT(rect.h, 10);
    }
    
}

TEST(FaceDetectionTest, FacePosition) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    int count = detect("single_face.jpg", buf,bufsize);
    EXPECT_EQ(count,1);
}

TEST(FaceDetectionTest, ZeroBuffSize) {
    constexpr int bufsize = 0;
    FaceRect buf[bufsize];
    EXPECT_EQ(detect("single_face.jpg", buf,bufsize), 0);
}

TEST(FaceDetectionTest, MultiFaceSmallBuff) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(detect("many_faces.jpg", buf,i),i);
    }
}

TEST(FaceDetectionTest, NoFaces) {
    constexpr int bufsize = 100;
    FaceRect buf[bufsize];
    EXPECT_EQ(detect("no_faces.jpg", buf,bufsize),0);
}