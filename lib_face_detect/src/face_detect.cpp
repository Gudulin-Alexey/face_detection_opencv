#include "face_detect_api.h"

#include <iostream>

#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "yunet_2023mar.h"


const uchar* model_buffer = face_detection_yunet_2023mar_onnx;          //Using header stile loading model file
const size_t model_buffer_size = face_detection_yunet_2023mar_onnx_len; //so no need to select and load it in run time and makes library sufficient

const std::vector<uchar>& GetModelData() {
    static const std::vector<uchar> model_data_vector(model_buffer,model_buffer + model_buffer_size);
    return model_data_vector;
}

const std::vector<uchar>& GetModelConfig() {
    static const std::vector<uchar> model_config_vector;
    return model_config_vector;
}

cv::Ptr<cv::FaceDetectorYN>& GetThreadLocalDetector() {
    thread_local cv::Ptr<cv::FaceDetectorYN> thread_local_detector;
    const int backend_id = 0;           // DNN backend for model by default opencv
    const int target_id = 0;            // DNN target where to run 0=cpu
    const float score_threshold = 0.9;  // Minimum confidence for model to identify a face
    const float nms_threshold = 0.3;    // Threshold to suppress overlapped boxes
    const int top_k = 5000;             // Keep top_k bounding boxes before NMS
    if (thread_local_detector.empty()) {
        thread_local_detector = cv::FaceDetectorYN::create(
            "onnx",
            GetModelData(),
            GetModelConfig(),
            cv::Size(320,320),
            score_threshold,
            nms_threshold,
            top_k,
            backend_id,
            target_id
        );
    }
    return thread_local_detector;
}

int detect(const char* img_path, FaceRect* face_rect_buf, int buf_size)
{
    cv::Mat faces;
    try {
        cv::Ptr<cv::FaceDetectorYN> detector = GetThreadLocalDetector(); // Use thread_local FaceDetectorYN insted of static so it can be thread save
        cv::Mat img = cv::imread(img_path);

        detector->setInputSize(img.size()); // Set input size for model instead of resizing img
        detector->setTopK(buf_size);        // Set max detected boxes

        detector->detect(img, faces);
    } catch (cv::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Error: "  << e.what() << std::endl;
        return -1;
    }
    for (int i=0; i < faces.rows && i < buf_size; i++) {
        //Fill boundong boxes of detected faces
        face_rect_buf[i].x1 = static_cast<int>(faces.at<float>(i, 0));
        face_rect_buf[i].y1 = static_cast<int>(faces.at<float>(i, 1));
        face_rect_buf[i].w = static_cast<int>(faces.at<float>(i, 2));
        face_rect_buf[i].h = static_cast<int>(faces.at<float>(i, 3));
    }
    return faces.rows;
}