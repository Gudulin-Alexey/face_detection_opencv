#include "application.h"
#include <dlfcn.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>

Application::Application(const std::string& root) : root_path_(root) {
    if (!fs::is_directory(root_path_)) {
        throw std::runtime_error("Path is not a directory");
    }
    LoadSharedLibrary();
}

Application::~Application() {
    if(lib_handle_) {
        dlclose(lib_handle_);
    }
}
void Application::LoadSharedLibrary() {
    const std::string lib_path = "./libface_detect.so";
    lib_handle_ = dlopen(lib_path.c_str(),RTLD_NOW);
    if (!lib_handle_) {
        throw std::runtime_error("Failed to load library");
    }
    detect_face_ptr_ = reinterpret_cast<DetectFacesFunc>(dlsym(lib_handle_,"detect"));
    if(!detect_face_ptr_) {
        throw std::runtime_error("Failed to find function 'detect' in library");
    }
}

void Application::FindImageFiles() {
    const std::vector<std::string> image_extentions = {".jpg", ".jpeg", ".jpe", ".png", ".bmp", ".jp2", ".ppm", ".sr", ".ras", ".tiff", ".tif"};
    for (const auto& entry : fs::recursive_directory_iterator(root_path_)) {
        if (entry.is_regular_file()) {
            std::string extention = entry.path().extension();
            std::transform(extention.begin(),                   //file extention is case insesetive
                extention.end(),                                // convert extention to lowercase
                extention.begin(),
                [](unsigned char c){ return std::tolower(c);}
            );
            bool is_img = false;
            for (const auto& ext : image_extentions) {
                if (ext == extention) {
                    is_img = true;
                    break;
                }
            }
            if (is_img) {
                image_paths_.push_back(entry);
            }
        }
    }
}

void Application::ProcessImages() {
    for(const auto& img_path : image_paths_) {
        std::cout << "img:" << img_path << std::endl;
        int count = detect_face_ptr_(img_path.c_str(), faceBuf_, sizeof(faceBuf_)/sizeof(FaceRect));
    }
}

void Application::Run() {
    FindImageFiles();
    ProcessImages();
}