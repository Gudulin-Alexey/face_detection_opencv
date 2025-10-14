#include "application.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_set>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

Application::Application(const AppConfig& config) : config_(config) {
    if (!fs::is_directory(config_.root_path) || !fs::is_directory(config_.output_path)) {
        throw std::runtime_error("Path is not a directory");
    }
    LoadSharedLibrary();
}

Application::~Application() {
    if(lib_handle_) {
        #ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
        #else
        dlclose(lib_handle_);
        #endif
    }
}
void Application::LoadSharedLibrary() {

    #ifdef _WIN32
    lib_handle_ = LoadLibrary(config_.library_path.c_str());
    #else
    lib_handle_ = dlopen(config_.library_path.c_str(),RTLD_NOW);
    #endif
    if (!lib_handle_) {
        throw std::runtime_error("Failed to load library");
    }
    #ifdef _WIN32
    detect_face_ptr_ = reinterpret_cast<DetectFacesFunc>(GetProcAddress(static_cast<HMODULE>(lib_handle_), "detect"));
    #else
    detect_face_ptr_ = reinterpret_cast<DetectFacesFunc>(dlsym(lib_handle_,"detect"));
    #endif
    if(!detect_face_ptr_) {
        throw std::runtime_error("Failed to find function 'detect' in library");
    }
}

std::string ToLowCase(const std::string& s) {
    std::string lower_str = s;
    std::transform(lower_str.begin(),
        lower_str.end(),
        lower_str.begin(),
        [](unsigned char c){ return std::tolower(c);}
    );
    return lower_str;
}

void Application::FindImageFiles() {
    const std::unordered_set<std::string> image_extentions{".jpg", ".jpeg", ".jpe", ".png", ".bmp", ".jp2", ".ppm", ".sr", ".ras", ".tiff", ".tif"};
    for (const auto& entry : fs::recursive_directory_iterator(config_.root_path)) {
        if (entry.is_regular_file() && !fs::is_empty(entry)) {
            std::string extention = ToLowCase(entry.path().extension());
            if (image_extentions.find(extention) != image_extentions.end())
                image_paths_.push_back(entry);
        }
    }
}
void Application::ProcessImageWorker() {
    std::cout <<"start new thread"<<std::endl;
    while (1) {
        int id = unique_img_id_.fetch_add(1);
        if (id >= image_paths_.size())
            return;
        fs::path img_path = image_paths_[id];
        Json::Value result = ProcessOneImage(img_path, id);
        std::lock_guard<std::mutex> lock(processed_results_mtx_);
        processed_results_["processed_images"].append(result);
    }
}

void Application::WriteResultsToFile() {
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::string result_file_name(config_.output_path.string());
    result_file_name.append("result.json");
    std::ofstream result_file(result_file_name);
    writer->write(processed_results_, &result_file);
}

void Application::ProcessImagesMultiTrhead() {
    processed_results_["processed_images"] = Json::arrayValue;
    std::vector<std::thread> worker_threads;
    for (int i = 0; i < config_.thread_num; i++) {
        worker_threads.emplace_back(std::thread(&Application::ProcessImageWorker, this));
    }
    for (auto& thread : worker_threads) {
        thread.join();
    }
}


void Application::ProcessImages() {
    //using stream writer so not needed to store whole dom in memory in case we parse large amount of images
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::string result_file_name(config_.output_path.string());
    result_file_name.append("result.json");
    std::ofstream result_file(result_file_name);
    result_file << "{\n\"processed_images\": [\n";      //direct write to open json brackets
    bool is_first = true;
    for(const auto& img_path : image_paths_) {
        if (is_first) {
            is_first = false;
        } else {
            result_file << ",\n";         // add comma between json objects
        }

        Json::Value val = ProcessOneImage(img_path, ++unique_img_id_);
        if (!val.empty())
            writer->write(val, &result_file);

    }
    result_file << "]\n}\n";                            //direct write to close json brackets

}
void BlureImageRect(cv::Mat& img, const cv::Rect& face_rect, float factor) {
    
    int kW = face_rect.width / factor;
    int kH = face_rect.height / factor;

    if (kW % 2 == 0)          //kWidth and kHeight should be odd
        kW--;
    if (kH % 2 == 0)
        kH--;
    //using gausing blur but it will not fit for securing data, probably bettere use pixelate
    cv::GaussianBlur(img(face_rect), img(face_rect), cv::Size(kW,kH), 0);
}

Json::Value Application::ProcessOneImage(const fs::path& img_path, int unique_id) {
    // call detec function from dinamicly loaded library
    FaceRect faceBuf[MAX_FACES_PER_IMAGE];
    int count = detect_face_ptr_(img_path.c_str(), faceBuf, sizeof(faceBuf)/sizeof(FaceRect));
    Json::Value val;
    if (count < 0)
        return val;
    val["faces"] = Json::arrayValue;
    for (int i = 0; i < count; i++) {
        // fill json object for every tetected face
        Json::Value detection;
        detection["x"] = faceBuf[i].x;
        detection["y"] = faceBuf[i].y;
        detection["witdh"] = faceBuf[i].w;
        detection["heigh"] = faceBuf[i].h;
        val["faces"].append(detection);
    }
    val["original_img"] = img_path.c_str();
    val["face_count"] = count;
    try {
        cv::Mat img = cv::imread(img_path.c_str());
        cv::Mat processed_img;
        cv::Size img_size = img.size();
        if (img_size.width > 5 && img_size.height > 5) { //make assumption that there is no faces on such small img and we do not resize it further 
            //resizing image with scale factor 0.5 using INTER_AREA since it looks best for shrinking
            cv::resize(img, processed_img, cv::Size(), config_.resize_scale, config_.resize_scale, cv::INTER_AREA);
            
            for (int i = 0; i < count; i++) {
                //scale detected face rectangle to new image
                cv::Rect blur_rect(faceBuf[i].x * config_.resize_scale, faceBuf[i].y * config_.resize_scale, faceBuf[i].w * config_.resize_scale, faceBuf[i].h * config_.resize_scale);
                //blur every rectangle with face
                BlureImageRect(processed_img, blur_rect, BLUR_KERNEL_DIV);
            }
        } else {
            processed_img = img.clone();
        }

        //compose processed img name using static counter
        std::string processed_img_path(config_.output_path.string());
        processed_img_path.append("processed_");

        processed_img_path.append(std::to_string(unique_id));
        processed_img_path.append(".jpg");

        cv::imwrite(processed_img_path, processed_img);
        val["processed_img"] = processed_img_path;
    } catch (const std::exception& e) {
        std::cerr << "Error:" << e.what() << std::endl;
        std::cerr << "ImgName:" << img_path.c_str() << std::endl;
    }
    return val;
}

void Application::Run() {
    FindImageFiles();
    ProcessImagesMultiTrhead();
    WriteResultsToFile();
}