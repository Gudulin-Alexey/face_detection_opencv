#include <string>
#include <dlfcn.h>
#include <iostream>
#include <opencv2/core/utility.hpp>

#include "application.h"

int main(int argc, char** argv) {
    const std::string keys =
    "{help h usage ?   |      | print this message   }"
    "{folder f         |./    | path to working folder to find images}"
    "{output o         |      | folder where to store results by defult equial to folder arg}"
    "{max_face_count N |100   | max number of faces on image}"
    ;
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")){
        parser.printMessage();
        return 0;
    }
    int N = parser.get<int>("max_face_count");
    std::string root_path = parser.get<std::string>("folder");
    std::string out_path = parser.get<std::string>("output");
    if (out_path.empty()) {
        out_path = root_path;
    }
    
    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }
    Application app("./");
    app.Run();
    
}
