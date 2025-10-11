#include <string>
#include <dlfcn.h>
#include <iostream>

#include "face_detect_api.h"

using MyFunc = int (*)(const char*);
void* load_library(const std::string& lib_path){
    return dlopen(lib_path.c_str(),RTLD_NOW); //we use RTLD_NOW since will use all symbols
}

void* get_function(void* handle, const std::string& func_name) {
    return dlsym(handle,func_name.c_str());
}
int main(){
    const std::string lib_path = "./libface_detect.so";
    void* lib_handle = load_library(lib_path);
    if (!lib_handle) {
        std::cout<< "Error: failed to load library"<<std::endl;
        return 1;
    }
    MyFunc f_ptr = reinterpret_cast<MyFunc>(get_function(lib_handle,"detect"));
    std::cout<<f_ptr("")<<std::endl;
    
}
