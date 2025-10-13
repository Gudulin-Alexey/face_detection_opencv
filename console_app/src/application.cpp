#include "application.h"
#include <dlfcn.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>


Application::Application(const AppConfig& config) : config_(config) {
    if (!fs::is_directory(config_.root_path)) {
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
    lib_handle_ = dlopen(config_.library_path.c_str(),RTLD_NOW);
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
    for (const auto& entry : fs::recursive_directory_iterator(config_.root_path)) {
        if (entry.is_regular_file() && !fs::is_empty(entry)) {
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
    //using stream writer so not needed to store whole dom in memory in case we parse large amount of images
    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::string result_file_name(config_.root_path.string());
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

        Json::Value val = ProcessOneImage(img_path);

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

Json::Value Application::ProcessOneImage(const fs::path& img_path) {
    // call detec function from dinamicly loaded library
    int count = detect_face_ptr_(img_path.c_str(), faceBuf_, sizeof(faceBuf_)/sizeof(FaceRect));
    Json::Value val;
    val["faces"] = Json::arrayValue;
    for (int i = 0; i < count; i++) {
        // fill json object for every tetected face
        Json::Value detection;
        detection["x"] = faceBuf_[i].x1;
        detection["y"] = faceBuf_[i].y1;
        detection["witdh"] = faceBuf_[i].w;
        detection["heigh"] = faceBuf_[i].h;
        val["faces"].append(detection);
    }
    val["original_img"] = img_path.c_str();
    val["face_count"] = count;
    try {
        cv::Mat img = cv::imread(img_path.c_str());
        cv::Mat processed_img;
        //resizing image with scale factor 0.5 using INTER_AREA since it looks best for shrinking
        cv::resize(img, processed_img, cv::Size(), config_.resize_scale, config_.resize_scale, cv::INTER_AREA);
        
        for (int i = 0; i < count; i++) {
            //scale detected face rectangle to new image
            cv::Rect blur_rect(faceBuf_[i].x1 * config_.resize_scale, faceBuf_[i].y1 * config_.resize_scale, faceBuf_[i].w * config_.resize_scale, faceBuf_[i].h * config_.resize_scale);
            //blur every rectangle with face
            BlureImageRect(processed_img, blur_rect, BLUR_KERNEL_DIV);
        }

        //compose processed img name using static counter
        std::string processed_img_path(config_.root_path.string());
        processed_img_path.append("processed_");

        processed_img_path.append(std::to_string(++unique_img_id_));
        processed_img_path.append(".jpg");

        cv::imwrite(processed_img_path, processed_img);
        val["processed_img"] = processed_img_path;
    } catch (const std::exception& e) {
        std::cerr << "Error:" << e.what() << std::endl;
    }
    return val;
}

void Application::Run() {
    FindImageFiles();
    ProcessImages();
}