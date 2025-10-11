#include "application.h"
#include <dlfcn.h>
#include <stdexcept>
#include <iostream>

Application::Application(const std::string& root) : root_path_(root) {

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

}

void Application::ProcessImages() {
    int count = detect_face_ptr_("", faceBuf_, sizeof(faceBuf_)/sizeof(FaceRect));
    std::cout<<"faces count "<<count<<std::endl;
}

void Application::Run() {
    LoadSharedLibrary();
    FindImageFiles();
    ProcessImages();
}