#ifndef FACE_DETECT_API_H
#define FACE_DETECT_API_H

struct FaceRect{
    int x1;
    int y1;
    int x2;
    int y2;
};
extern "C"{

    int detect(const char* img_path, FaceRect* face_rect_buf, int buf_size);

}
#endif //FACE_DETECT_API_H
