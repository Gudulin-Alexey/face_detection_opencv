#ifndef FACE_DETECT_API_H
#define FACE_DETECT_API_H

#ifdef _WIN32
    #ifdef FACE_DETECT_EXPORT
        #define FACE_DETECT_API __declspec(dllexport)
    #else
        #define FACE_DETECT_API __declspec(dllimport)
    #endif
#else 
    #define FACE_DETECT_API __attribute__((visibility("default")))
#endif
struct FaceRect{
    int x;
    int y;
    int w;
    int h;
};
extern "C"{

    FACE_DETECT_API int detect(const char* img_path, FaceRect* face_rect_buf, int buf_size);

}
#endif //FACE_DETECT_API_H
