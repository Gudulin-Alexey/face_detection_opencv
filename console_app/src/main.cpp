#include <string>
#include <dlfcn.h>
#include <iostream>

#include "application.h"

int main() {
    Application app("./");
    app.Run();
    
}
