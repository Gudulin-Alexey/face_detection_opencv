#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <filesystem>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

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
    unsigned int thread_num = 1;
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
    void ProcessImagesMultiTrhead();
    void ProcessImageWorker();
    void WriteResultsToFile();
    Json::Value ProcessOneImage(const fs::path&, int);

    
    const AppConfig config_;
    std::vector<fs::path> image_paths_;
    
    static constexpr int MAX_FACES_PER_IMAGE = 100;
    static constexpr float BLUR_KERNEL_DIV = 2.0f;

    std::atomic<int> unique_img_id_ = 0;
    std::mutex processed_results_mtx_;
    Json::Value processed_results_;

    void* lib_handle_ = nullptr;
    DetectFacesFunc detect_face_ptr_ = nullptr;
};
#endif //APPLICATION_H