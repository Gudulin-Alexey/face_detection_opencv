#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <filesystem>
#include <vector>

#include "face_detect_api.h"

namespace fs = std::filesystem;
using DetectFacesFunc = int (*)(const char*, FaceRect*, int);
class Application
{
public:
    explicit Application(const std::string& root_path);
    ~Application();
    void Run();
private:
    void LoadSharedLibrary();
    void FindImageFiles();
    void ProcessImages();

    fs::path root_path_;
    std::vector<fs::path> image_paths_;

    void* lib_handle_ = nullptr;
    DetectFacesFunc detect_face_ptr_ = nullptr;
    FaceRect faceBuf_[100];
};
#endif //APPLICATION_H