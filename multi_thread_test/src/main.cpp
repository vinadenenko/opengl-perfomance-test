#include <iostream>
#include "multi_thread_app.h"

int main() {
    std::cout << "=== OpenGL Multi-Thread Performance Test ===" << std::endl;
    std::cout << "This test will demonstrate multi-threaded terrain rendering performance." << std::endl;
    std::cout << "The application uses a second OpenGL context for background rendering." << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  P - Toggle performance info" << std::endl;
    std::cout << "  W - Toggle wireframe mode" << std::endl;
    std::cout << "  L - Toggle lighting" << std::endl;
    std::cout << "  I - Toggle instancing" << std::endl;
    std::cout << "  M - Toggle multi-threading (runtime comparison)" << std::endl;
    std::cout << "  +/- - Scale terrain" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << std::endl;
    
    MultiThreadApp app(1280, 720);
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }
    
    return app.run();
}