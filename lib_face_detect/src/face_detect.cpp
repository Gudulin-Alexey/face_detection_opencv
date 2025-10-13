#include "face_detect_api.h"

#include <iostream>

#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "yunet_2023mar.h"


const uchar* model_buffer = face_detection_yunet_2023mar_onnx;          //Using header stile loading model file
const size_t model_buffer_size = face_detection_yunet_2023mar_onnx_len; //so no need to select and load it in run time and makes library sufficient

static constexpr float SCORE_THRESHOLD = 0.9;   // Minimum confidence for model to identify a face
static constexpr float NMS_THRESHOLD = 0.3;     // Threshold to suppress overlapped boxes
static constexpr const int BACKEND_ID = 0;           // DNN backend for model by default opencv
static constexpr const int TARGET_ID = 0;            // DNN target where to run 0=cpu
static constexpr const int TOP_K = 5000;            // Maximum boxes detected by model
static constexpr std::string_view FRAMEWORK = "onnx";

static constexpr const float DEFAULT_SCALE_FACTOR = 3.0;            // Maximum boxes detected by model
static constexpr std::pair<int,int> MAX_DETECTED_SIZE = {320,320};


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
    if (thread_local_detector.empty()) {
        thread_local_detector = cv::FaceDetectorYN::create(
            FRAMEWORK.data(),
            GetModelData(),
            GetModelConfig(),
            cv::Size(320,320),
            SCORE_THRESHOLD,
            NMS_THRESHOLD,
            TOP_K,
            BACKEND_ID,
            TARGET_ID
        );
    }
    return thread_local_detector;
}

int detect(const char* img_path, FaceRect* face_rect_buf, int buf_size)
{
    if (buf_size == 0 || face_rect_buf == NULL)
        return 0;
    try {
        int faces_count = 0;

        cv::Ptr<cv::FaceDetectorYN> detector = GetThreadLocalDetector(); // Use thread_local FaceDetectorYN insted of static so it can be thread save
        cv::Mat img = cv::imread(img_path);
        //since yunet model can detect faces from 10x10 to 320x320 in case of huge face on image need to resize it to smaller size 
        cv::Mat resized;
        float current_scale = 1;

        //Select scale factor if image is biger 3 times more the scale factor is 3 if less then scale direct to  MAX_DETECTED_SIZE
        float scale_factor = std::min(DEFAULT_SCALE_FACTOR,
            std::max(static_cast<float>(img.size().width) / MAX_DETECTED_SIZE.first,
            static_cast<float>(img.size().height) / MAX_DETECTED_SIZE.second)
        );

        std::vector<cv::Rect> all_found_boxes;
        std::vector<float> all_confidence;
        do {
            cv::resize(img,resized,cv::Size(), current_scale, current_scale, cv::INTER_AREA);
            detector->setInputSize(resized.size()); // Set input size for model so it can handle our img

            cv::Mat faces;
            detector->detect(resized, faces);
            for (int i=0; i < faces.rows; i++) {
                int x = static_cast<int>(faces.at<float>(i, 0));
                int y = static_cast<int>(faces.at<float>(i, 1));
                int w = static_cast<int>(faces.at<float>(i, 2));
                int h = static_cast<int>(faces.at<float>(i, 3));

                float confidence = faces.at<float>(i, 14);
                //construct rectangle of found face box according to current scale
                cv::Rect face_box(
                    static_cast<int>(x/current_scale),
                    static_cast<int>(y/current_scale),
                    static_cast<int>(w/current_scale),
                    static_cast<int>(h/current_scale)
                );
                all_found_boxes.push_back(face_box);        //save all found rectangles
                all_confidence.push_back(confidence);
            }
            current_scale = current_scale / scale_factor;
            //do not continue if already reached MAX_DETECTED_SIZE
            //or scale_factor < 1 means that img is smaller then MAX_DETECTED_SIZE
        } while (resized.size().width > MAX_DETECTED_SIZE.first && resized.size().height > MAX_DETECTED_SIZE.second && scale_factor > 1);

        //vector for storing indexes of unique boxes
        std::vector<int> final_indices;
        //remove duplicates of same detected faces on differnt scales
        cv::dnn::NMSBoxes(all_found_boxes, all_confidence, SCORE_THRESHOLD, NMS_THRESHOLD, final_indices);

        faces_count = std::min(static_cast<int>(final_indices.size()), buf_size);
        for (int i=0; i < faces_count; i++) {
            //Fill boundong boxes of detected faces for output
            cv::Rect final_box = all_found_boxes[final_indices[i]];
            face_rect_buf[i].x1 = final_box.x;
            face_rect_buf[i].y1 = final_box.y;
            face_rect_buf[i].w = final_box.width;
            face_rect_buf[i].h = final_box.height;
        }

        std::cout<<"Img:"<<img_path<<std::endl;
        std::cout<<"Found faces count: "<<faces_count<<std::endl;
        return faces_count;
    } catch (const std::exception& e) {
        std::cerr << "Error: "  << e.what() << std::endl;
        return -1;
    }
}