#include <iostream>
#include "single_thread_app.h"

int main() {
    std::cout << "=== OpenGL Single-Thread Performance Test ===" << std::endl;
    std::cout << "This test will demonstrate single-threaded terrain rendering performance." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  P - Toggle performance info" << std::endl;
    std::cout << "  W - Toggle wireframe mode" << std::endl;
    std::cout << "  L - Toggle lighting" << std::endl;
    std::cout << "  I - Toggle instancing (if available)" << std::endl;
    std::cout << "  +/- - Scale terrain" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << std::endl;
    
    SingleThreadApp app(1280, 720);
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }
    
    return app.run();
}