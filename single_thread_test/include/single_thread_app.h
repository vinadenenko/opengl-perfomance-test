#pragma once

// Include GLEW first (via gl_utils.h) before GLFW to avoid header conflicts
#include "gl_utils.h"
#include <GLFW/glfw3.h>
#include <memory>
#include "performance_monitor.h"
#include "terrain_generator.h"

class SingleThreadApp {
public:
    SingleThreadApp(int windowWidth = 1280, int windowHeight = 720);
    ~SingleThreadApp();
    
    bool initialize();
    int run();
    
private:
    // Initialization
    bool initializeGL();
    bool initializeTerrain();
    bool initializeShaders();
    
public:
    // Configuration
    void configureTerrain(int gridSize, int patchCount, float heightScale);
    
private:
    
    // Main loop
    void update();
    void render();
    void handleInput();
    
    // Rendering functions
    void uploadPatchToGPU(TerrainPatch& patch);
    void renderPatch(const TerrainPatch& patch);
    void setupMatrices();
    
    // Cleanup
    void cleanup();
    
    // Member variables
    GLFWwindow* window_;
    int windowWidth_;
    int windowHeight_;
    
    // Terrain
    std::unique_ptr<TerrainGenerator> terrainGenerator_;
    
    // Shaders
    Shader terrainShader_;
    
    // Camera
    glm::vec3 cameraPos_;
    glm::vec3 cameraFront_;
    glm::vec3 cameraUp_;
    float cameraYaw_;
    float cameraPitch_;
    float cameraSpeed_;
    float mouseSensitivity_;
    bool firstMouse_;
    double lastMouseX_;
    double lastMouseY_;
    
    // Matrices
    glm::mat4 view_;
    glm::mat4 projection_;
    
    // Performance
    std::unique_ptr<PerformanceMonitor> perfMonitor_;
    bool showPerformanceInfo_;
    
    // Timing
    float deltaTime_;
    float lastFrame_;
    
    // Controls
    bool wireframeMode_;
    bool useLighting_;
    bool useInstancing_;
    float globalScale_;
    
    // GLFW callbacks (static)
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // User data pointer for callbacks
    static SingleThreadApp* s_instance_;
};
