#include <string>
#include <iostream>
#include <opencv2/core/utility.hpp>

#include "application.h"

int main(int argc, char** argv) {
    const std::string keys =
    "{help h usage ?   | | dont forget '=' when pasing arguments for example -j=4   }"
    "{@input           | | (Required) path to working folder to find images}"
    "{output o         | | (Optional) folder where to store results by defult equial to folder arg}"
    "{threads j        |1| (Optional) number of threads to run app}"
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
        std::string root_path = parser.get<std::string>("@input");
        std::string out_path = parser.get<std::string>("output");
        int num_of_threads = parser.get<int>("threads");

        if (root_path.empty()) {
            root_path = parser.getPathToApplication();
        }
        if (out_path.empty() || out_path == "true") {
            out_path = root_path;
        }
        AppConfig config;
        config.root_path = root_path;
        config.output_path = out_path;
        if (num_of_threads > 0)
            config.thread_num = num_of_threads;
        Application app(config);
        app.Run();
    } catch(const std::exception& e){
        std::cerr << "Error:" << e.what() << std::endl;
        return 1;
    }
    std::cout << "Application finished succesfully" << std::endl;
    return 0;
}
