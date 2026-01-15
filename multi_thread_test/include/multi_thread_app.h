#pragma once

// Include GLEW first (via gl_utils.h) before GLFW to avoid header conflicts
#include "gl_utils.h"
#include <GLFW/glfw3.h>
#include <memory>
#include "terrain_generator.h"
#include "performance_monitor.h"
#include "render_thread.h"

class MultiThreadApp {
public:
    MultiThreadApp(int windowWidth = 1280, int windowHeight = 720);
    ~MultiThreadApp();
    
    bool initialize();
    int run();
    
private:
    // Initialization
    bool initializeGL();
    bool initializeTerrain();
    bool initializeShaders();
    bool initializeRenderThread();
    
    // Main loop
    void update();
    void render();
    void handleInput();
    
    // Rendering functions
    void uploadPatchToGPU(TerrainPatch& patch);
    void renderPatch(const TerrainPatch& patch);
    void setupMatrices();
    void distributeRenderWork();
    
    // Multi-threading utilities
    void submitPatchUploadWork();
    void waitForRenderThread();
    
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
    Shader instancedShader_;
    
    // Render thread
    std::unique_ptr<RenderThread> renderThread_;
    
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
    std::unique_ptr<PerformanceMonitor> renderThreadPerfMonitor_;
    bool showPerformanceInfo_;
    
    // Timing
    float deltaTime_;
    float lastFrame_;
    
    // Controls
    bool wireframeMode_;
    bool useLighting_;
    bool useInstancing_;
    float globalScale_;
    
    // Multi-threading state
    bool useMultiThreading_;
    std::atomic<int> patchesUploaded_;
    std::atomic<int> totalPatches_;
    
    // GLFW callbacks (static)
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // User data pointer for callbacks
    static MultiThreadApp* s_instance_;
};