#include "multi_thread_app.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Static member initialization
MultiThreadApp* MultiThreadApp::s_instance_ = nullptr;

MultiThreadApp::MultiThreadApp(int windowWidth, int windowHeight)
    : window_(nullptr), windowWidth_(windowWidth), windowHeight_(windowHeight),
      cameraPos_(0.0f, 5.0f, 10.0f), cameraFront_(0.0f, 0.0f, -1.0f),
      cameraUp_(0.0f, 1.0f, 0.0f), cameraYaw_(-90.0f), cameraPitch_(0.0f),
      cameraSpeed_(5.0f), mouseSensitivity_(0.1f), firstMouse_(true),
      lastMouseX_(windowWidth_ / 2.0), lastMouseY_(windowHeight_ / 2.0),
      globalScale_(1.0f), deltaTime_(0.0f), lastFrame_(0.0f),
      wireframeMode_(false), useLighting_(true), useInstancing_(false),
      showPerformanceInfo_(true), useMultiThreading_(true),
      patchesUploaded_(0), totalPatches_(0) {
    s_instance_ = this;
}

MultiThreadApp::~MultiThreadApp() {
    cleanup();
}

bool MultiThreadApp::initialize() {
    std::cout << "Initializing Multi-Thread Application..." << std::endl;
    
    if (!initializeGL()) {
        std::cerr << "Failed to initialize OpenGL!" << std::endl;
        return false;
    }
    
    if (!initializeTerrain()) {
        std::cerr << "Failed to initialize terrain!" << std::endl;
        return false;
    }
    
    if (!initializeShaders()) {
        std::cerr << "Failed to initialize shaders!" << std::endl;
        return false;
    }
    
    if (!initializeRenderThread()) {
        std::cerr << "Failed to initialize render thread!" << std::endl;
        return false;
    }
    
    perfMonitor_ = std::make_unique<PerformanceMonitor>();
    renderThreadPerfMonitor_ = std::make_unique<PerformanceMonitor>();
    
    std::cout << "Multi-Thread Application initialized successfully!" << std::endl;
    return true;
}

bool MultiThreadApp::initializeGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window_ = glfwCreateWindow(windowWidth_, windowHeight_, "Multi-Thread OpenGL Test", NULL, NULL);
    if (!window_) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    
    // Initialize GLEW immediately after OpenGL context is created
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW! Error: " << glewGetErrorString(glewError) << std::endl;
        return false;
    }
    
    // Check if GLEW initialized successfully
    if (!glewIsSupported("GL_VERSION_3_3")) {
        std::cerr << "OpenGL 3.3 not supported!" << std::endl;
        return false;
    }
    
    std::cout << "GLEW initialized successfully!" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, mouseCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    return true;
}

bool MultiThreadApp::initializeTerrain() {
    terrainGenerator_ = std::make_unique<TerrainGenerator>(256, 1.0f, 20.0f);
    terrainGenerator_->generateTerrain();
    totalPatches_ = terrainGenerator_->getPatches().size();
    return true;
}

void MultiThreadApp::configureTerrain(int gridSize, int patchCount, float heightScale) {
    if (terrainGenerator_) {
        terrainGenerator_->setGridSize(gridSize);
        terrainGenerator_->setPatchCount(patchCount);
        terrainGenerator_->setHeightScale(heightScale);
        terrainGenerator_->generateTerrain();
        totalPatches_ = terrainGenerator_->getPatches().size();
    }
}

bool MultiThreadApp::initializeShaders() {
    terrainShader_.load("shaders/basic.vert", "shaders/terrain.frag");
    if (!terrainShader_.isValid()) {
        std::cerr << "Failed to load terrain shader!" << std::endl;
        return false;
    }
    
    instancedShader_.load("shaders/instanced.vert", "shaders/terrain.frag");
    if (!instancedShader_.isValid()) {
        std::cout << "Warning: Instanced shader failed to load, instancing disabled" << std::endl;
        useInstancing_ = false;
    }
    
    return true;
}

bool MultiThreadApp::initializeRenderThread() {
    renderThread_ = std::make_unique<RenderThread>();
    
    if (!renderThread_->initialize(window_)) {
        std::cerr << "Failed to initialize render thread!" << std::endl;
        return false;
    }
    
    renderThread_->start();
    return true;
}

int MultiThreadApp::run() {
    setupMatrices();
    
    // Upload terrain patches in background
    if (useMultiThreading_) {
        submitPatchUploadWork();
    }
    
    while (!glfwWindowShouldClose(window_)) {
        float currentFrame = glfwGetTime();
        deltaTime_ = currentFrame - lastFrame_;
        lastFrame_ = currentFrame;
        
        perfMonitor_->beginFrame();
        
        update();
        render();
        handleInput();
        
        perfMonitor_->endFrame();
        
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
    
    if (showPerformanceInfo_) {
        perfMonitor_->printReport();
        
        if (useMultiThreading_) {
            std::cout << "\n=== Render Thread Performance ===" << std::endl;
            renderThreadPerfMonitor_->printReport();
            std::cout << "Processed Tasks: " << renderThread_->getProcessedTasks() << std::endl;
        }
    }
    
    return 0;
}

void MultiThreadApp::update() {
    // Update camera position based on input
    // (handled in handleInput)
}

void MultiThreadApp::render() {
    if (wireframeMode_) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    // Use appropriate shader
    if (useInstancing_ && instancedShader_.isValid()) {
        instancedShader_.use();
        instancedShader_.setMat4("view", view_);
        instancedShader_.setMat4("projection", projection_);
        instancedShader_.setFloat("globalScale", globalScale_);
    } else {
        terrainShader_.use();
        terrainShader_.setMat4("view", view_);
        terrainShader_.setMat4("projection", projection_);
        terrainShader_.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(globalScale_)));
    }
    
    // Set common uniforms
    Shader& currentShader = useInstancing_ ? instancedShader_ : terrainShader_;
    currentShader.setVec3("lightPos", glm::vec3(50.0f, 50.0f, 50.0f));
    currentShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    currentShader.setVec3("viewPos", cameraPos_);
    currentShader.setInt("useLighting", useLighting_ ? 1 : 0);
    currentShader.setInt("useTexture", 0);
    currentShader.setFloat("time", glfwGetTime());
    
    // Wait for render thread to complete uploads before rendering
    if (useMultiThreading_) {
        waitForRenderThread();
    }
    
    // Render terrain patches
    const auto& patches = terrainGenerator_->getPatches();
    for (const auto& patch : patches) {
        if (!useMultiThreading_) {
            // Single-threaded fallback
            uploadPatchToGPU(const_cast<TerrainPatch&>(patch));
        }
        
        renderPatch(patch);
        perfMonitor_->incrementDrawCalls();
        perfMonitor_->addTriangles(patch.indices.size() / 3);
        perfMonitor_->addVertices(patch.vertices.size());
    }
    
    // Display performance info
    if (showPerformanceInfo_) {
        // Simple text overlay could be implemented here
        // For now, we use console output periodically
    }
}

void MultiThreadApp::handleInput() {
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
    
    float cameraSpeed = cameraSpeed_ * deltaTime_;
    
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos_ += cameraSpeed * cameraFront_;
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos_ -= cameraSpeed * cameraFront_;
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos_ -= glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos_ += glm::normalize(glm::cross(cameraFront_, cameraUp_)) * cameraSpeed;
    }
    
    if (glfwGetKey(window_, GLFW_KEY_P) == GLFW_PRESS) {
        showPerformanceInfo_ = !showPerformanceInfo_;
    }
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        wireframeMode_ = !wireframeMode_;
    }
    if (glfwGetKey(window_, GLFW_KEY_L) == GLFW_PRESS) {
        useLighting_ = !useLighting_;
    }
    if (glfwGetKey(window_, GLFW_KEY_I) == GLFW_PRESS) {
        useInstancing_ = !useInstancing_;
    }
    if (glfwGetKey(window_, GLFW_KEY_M) == GLFW_PRESS) {
        useMultiThreading_ = !useMultiThreading_;
        std::cout << "Multi-threading " << (useMultiThreading_ ? "enabled" : "disabled") << std::endl;
    }
    if (glfwGetKey(window_, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        globalScale_ += 0.1f;
    }
    if (glfwGetKey(window_, GLFW_KEY_MINUS) == GLFW_PRESS) {
        globalScale_ -= 0.1f;
    }
    
    setupMatrices();
}

void MultiThreadApp::uploadPatchToGPU(TerrainPatch& patch) {
    if (!patch.isUploaded) {
        glGenVertexArrays(1, &patch.VAO);
        glGenBuffers(1, &patch.VBO);
        glGenBuffers(1, &patch.EBO);
        
        glBindVertexArray(patch.VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, patch.VBO);
        glBufferData(GL_ARRAY_BUFFER, patch.vertices.size() * sizeof(TerrainVertex), 
                    patch.vertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, patch.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, patch.indices.size() * sizeof(unsigned int),
                    patch.indices.data(), GL_STATIC_DRAW);
        
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 
                            (void*)offsetof(TerrainVertex, position));
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                            (void*)offsetof(TerrainVertex, normal));
        glEnableVertexAttribArray(1);
        
        // Texture coordinate attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                            (void*)offsetof(TerrainVertex, texCoord));
        glEnableVertexAttribArray(2);
        
        // Color attribute
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                            (void*)offsetof(TerrainVertex, color));
        glEnableVertexAttribArray(3);
        
        if (useInstancing_) {
            // Setup instanced attributes
            glVertexAttribDivisor(0, 0); // Per-vertex
            glVertexAttribDivisor(1, 0); // Per-vertex
            glVertexAttribDivisor(2, 0); // Per-vertex
            glVertexAttribDivisor(3, 0); // Per-vertex
        }
        
        glBindVertexArray(0);
        patch.isUploaded = true;
        patchesUploaded_++;
        
        perfMonitor_->addVBOMemory(patch.vertices.size() * sizeof(TerrainVertex) + 
                                  patch.indices.size() * sizeof(unsigned int));
    }
}

void MultiThreadApp::renderPatch(const TerrainPatch& patch) {
    if (patch.isUploaded) {
        glBindVertexArray(patch.VAO);
        
        if (useInstancing_) {
            // glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instances); // Example
            glDrawElements(GL_TRIANGLES, patch.indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawElements(GL_TRIANGLES, patch.indices.size(), GL_UNSIGNED_INT, 0);
        }
        
        glBindVertexArray(0);
    }
}

void MultiThreadApp::setupMatrices() {
    view_ = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);
    projection_ = glm::perspective(glm::radians(45.0f), 
                                  (float)windowWidth_ / (float)windowHeight_, 
                                  0.1f, 100.0f);
}

void MultiThreadApp::submitPatchUploadWork() {
    std::cout << "Submitting " << totalPatches_ << " patches to render thread..." << std::endl;
    
    const auto& patches = terrainGenerator_->getPatches();
    for (size_t i = 0; i < patches.size(); ++i) {
        renderThread_->submitPatchUpload(i, 
                                    (void*)patches[i].vertices.data(), 
                                    patches[i].vertices.size() * sizeof(TerrainVertex),
                                    (void*)patches[i].indices.data(),
                                    patches[i].indices.size() * sizeof(unsigned int));
    }
    
    std::cout << "All patch upload tasks submitted!" << std::endl;
}

void MultiThreadApp::waitForRenderThread() {
    if (useMultiThreading_ && renderThread_) {
        renderThread_->waitForCompletion();
    }
}

void MultiThreadApp::cleanup() {
    if (renderThread_) {
        renderThread_->stop();
    }
    
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

// GLFW callbacks
void MultiThreadApp::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (s_instance_) {
        s_instance_->windowWidth_ = width;
        s_instance_->windowHeight_ = height;
        s_instance_->setupMatrices();
    }
}

void MultiThreadApp::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_instance_) {
        if (s_instance_->firstMouse_) {
            s_instance_->lastMouseX_ = xpos;
            s_instance_->lastMouseY_ = ypos;
            s_instance_->firstMouse_ = false;
        }
        
        double xoffset = xpos - s_instance_->lastMouseX_;
        double yoffset = s_instance_->lastMouseY_ - ypos;
        
        s_instance_->lastMouseX_ = xpos;
        s_instance_->lastMouseY_ = ypos;
        
        xoffset *= s_instance_->mouseSensitivity_;
        yoffset *= s_instance_->mouseSensitivity_;
        
        s_instance_->cameraYaw_ += xoffset;
        s_instance_->cameraPitch_ += yoffset;
        
        if (s_instance_->cameraPitch_ > 89.0f) {
            s_instance_->cameraPitch_ = 89.0f;
        }
        if (s_instance_->cameraPitch_ < -89.0f) {
            s_instance_->cameraPitch_ = -89.0f;
        }
        
        glm::vec3 front;
        front.x = cos(glm::radians(s_instance_->cameraYaw_)) * cos(glm::radians(s_instance_->cameraPitch_));
        front.y = sin(glm::radians(s_instance_->cameraPitch_));
        front.z = sin(glm::radians(s_instance_->cameraYaw_)) * cos(glm::radians(s_instance_->cameraPitch_));
        s_instance_->cameraFront_ = glm::normalize(front);
        
        s_instance_->setupMatrices();
    }
}

void MultiThreadApp::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (s_instance_) {
        s_instance_->cameraSpeed_ += yoffset;
        if (s_instance_->cameraSpeed_ < 1.0f) {
            s_instance_->cameraSpeed_ = 1.0f;
        }
        if (s_instance_->cameraSpeed_ > 50.0f) {
            s_instance_->cameraSpeed_ = 50.0f;
        }
    }
}