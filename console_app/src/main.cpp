#include <string>
#include <dlfcn.h>
#include <iostream>
#include <opencv2/core/utility.hpp>

#include "application.h"

int main(int argc, char** argv) {
    const std::string keys =
    "{help h usage ?   |      | print this message   }"
    "{@input           |      | (Required) path to working folder to find images}"
    "{output o         |      | (Optional) folder where to store results by defult equial to folder arg}"
    "{max_face_count N |100   | (Optional) max number of faces on image}"
    ;
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }
    if (!parser.check())
    {
        parser.printErrors();
        return 1;
    }
    try {
        int N = parser.get<int>("max_face_count");
        std::string root_path = parser.get<std::string>("@input");
        std::string out_path = parser.get<std::string>("output");
        if (root_path.empty()) {
            root_path = fs::current_path().parent_path();
        }
        if (out_path.empty()) {
            out_path = root_path;
        }
        Application app(root_path);
        app.Run();
    } catch(const std::exception& e){
        std::cerr<<"Error:"<< e.what()<<std::endl;
        return 1;
    }
    std::cout<<"Application finished succesfully"<<std::endl;
    return 0;
}
