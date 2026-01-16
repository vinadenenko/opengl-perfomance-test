#include <iostream>
#include "single_thread_app.h"

int main(int argc, char* argv[]) {
    // Default configuration
    int windowWidth = 1280;
    int windowHeight = 720;
    int gridSize = 256;
    int patchCount = 64;
    float heightScale = 20.0f;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--width") {
            windowWidth = std::atoi(argv[++i]);
        } else if (arg == "--height") {
            windowHeight = std::atoi(argv[++i]);
        } else if (arg == "--grid-size") {
            gridSize = std::atoi(argv[++i]);
        } else if (arg == "--patches") {
            patchCount = std::atoi(argv[++i]);
        } else if (arg == "--height-scale") {
            heightScale = std::atof(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --width <pixels>     Window width (default: 1280)" << std::endl;
            std::cout << "  --height <pixels>    Window height (default: 720)" << std::endl;
            std::cout << "  --grid-size <size>  Terrain grid size (default: 256)" << std::endl;
            std::cout << "  --patches <count>    Number of terrain patches (default: 64)" << std::endl;
            std::cout << "  --height-scale <s>  Terrain height scale (default: 20.0)" << std::endl;
            return 0;
        }
    }
    
    std::cout << "=== OpenGL Single-Thread Performance Test ===" << std::endl;
    std::cout << "Configuration: " << gridSize << "x" << gridSize << " grid, " << patchCount << " patches" << std::endl;
    std::cout << "Window: " << windowWidth << "x" << windowHeight << std::endl;
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
    
    SingleThreadApp app(windowWidth, windowHeight);
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }
    
    // Configure terrain
    app.configureTerrain(gridSize, patchCount, heightScale);
    
    return app.run();
}