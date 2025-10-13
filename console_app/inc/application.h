#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <filesystem>
#include <vector>
#include <json/json.h>

#include "face_detect_api.h"

namespace fs = std::filesystem;
using DetectFacesFunc = int (*)(const char*, FaceRect*, int);
struct AppConfig {
    fs::path root_path;
    fs::path output_path;
#ifdef _WIN32
    std::string library_path = "face_detect.dll";
#else
    std::string library_path = "./libface_detect.so";
#endif
    std::string output_filename = "result.json";
    float resize_scale = 0.5;
};
class Application
{

public:
    explicit Application(const AppConfig&);
    ~Application();
    void Run();
private:
    void LoadSharedLibrary();
    void FindImageFiles();
    void ProcessImages();
    Json::Value ProcessOneImage(const fs::path&);

    
    const AppConfig config_;
    std::vector<fs::path> image_paths_;

    static constexpr int MAX_FACES_PER_IMAGE = 100;
    static constexpr float BLUR_KERNEL_DIV = 2.0f;

    int unique_img_id_ = 0;


    void* lib_handle_ = nullptr;
    DetectFacesFunc detect_face_ptr_ = nullptr;
    FaceRect faceBuf_[MAX_FACES_PER_IMAGE];
};
#endif //APPLICATION_H