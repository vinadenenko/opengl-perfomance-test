#include "single_thread_app.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Static member initialization
SingleThreadApp* SingleThreadApp::s_instance_ = nullptr;

SingleThreadApp::SingleThreadApp(int windowWidth, int windowHeight)
    : window_(nullptr), windowWidth_(windowWidth), windowHeight_(windowHeight),
      cameraPos_(0.0f, 5.0f, 10.0f), cameraFront_(0.0f, 0.0f, -1.0f),
      cameraUp_(0.0f, 1.0f, 0.0f), cameraYaw_(-90.0f), cameraPitch_(0.0f),
      cameraSpeed_(5.0f), mouseSensitivity_(0.1f), firstMouse_(true),
      lastMouseX_(windowWidth_ / 2.0), lastMouseY_(windowHeight_ / 2.0),
      globalScale_(1.0f), deltaTime_(0.0f), lastFrame_(0.0f),
      wireframeMode_(false), useLighting_(true), useInstancing_(false),
      showPerformanceInfo_(true) {
    s_instance_ = this;
}

SingleThreadApp::~SingleThreadApp() {
    cleanup();
}

bool SingleThreadApp::initialize() {
    std::cout << "Initializing Single-Thread Application..." << std::endl;
    
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
    
    perfMonitor_ = std::make_unique<PerformanceMonitor>();
    
    std::cout << "Single-Thread Application initialized successfully!" << std::endl;
    return true;
}

bool SingleThreadApp::initializeGL() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window_ = glfwCreateWindow(windowWidth_, windowHeight_, "Single-Thread OpenGL Test", NULL, NULL);
    if (!window_) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, mouseCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
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
    
    return true;
}

bool SingleThreadApp::initializeTerrain() {
    terrainGenerator_ = std::make_unique<TerrainGenerator>(256, 1.0f, 20.0f);
    terrainGenerator_->generateTerrain();
    return true;
}

bool SingleThreadApp::initializeShaders() {
    terrainShader_.load("shaders/basic.vert", "shaders/terrain.frag");
    if (!terrainShader_.isValid()) {
        std::cerr << "Failed to load terrain shader!" << std::endl;
        return false;
    }
    return true;
}

int SingleThreadApp::run() {
    setupMatrices();
    
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
    }
    
    return 0;
}

void SingleThreadApp::update() {
    // Update camera position based on input
    // (handled in handleInput)
}

void SingleThreadApp::render() {
    if (wireframeMode_) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    terrainShader_.use();
    
    // Set matrices
    terrainShader_.setMat4("view", view_);
    terrainShader_.setMat4("projection", projection_);
    terrainShader_.setMat4("model", glm::mat4(1.0f));
    
    // Set lighting
    terrainShader_.setVec3("lightPos", glm::vec3(50.0f, 50.0f, 50.0f));
    terrainShader_.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    terrainShader_.setVec3("viewPos", cameraPos_);
    terrainShader_.setInt("useLighting", useLighting_ ? 1 : 0);
    terrainShader_.setInt("useTexture", 0);
    terrainShader_.setFloat("time", glfwGetTime());
    
    // Render terrain patches
    const auto& patches = terrainGenerator_->getPatches();
    for (const auto& patch : patches) {
        uploadPatchToGPU(const_cast<TerrainPatch&>(patch));
        renderPatch(patch);
        perfMonitor_->incrementDrawCalls();
        perfMonitor_->addTriangles(patch.indices.size() / 3);
        perfMonitor_->addVertices(patch.vertices.size());
    }
}

void SingleThreadApp::handleInput() {
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
    if (glfwGetKey(window_, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        globalScale_ += 0.1f;
    }
    if (glfwGetKey(window_, GLFW_KEY_MINUS) == GLFW_PRESS) {
        globalScale_ -= 0.1f;
    }
    
    setupMatrices();
}

void SingleThreadApp::uploadPatchToGPU(TerrainPatch& patch) {
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
        
        glBindVertexArray(0);
        patch.isUploaded = true;
        
        perfMonitor_->addVBOMemory(patch.vertices.size() * sizeof(TerrainVertex) + 
                                  patch.indices.size() * sizeof(unsigned int));
    }
}

void SingleThreadApp::renderPatch(const TerrainPatch& patch) {
    if (patch.isUploaded) {
        glBindVertexArray(patch.VAO);
        glDrawElements(GL_TRIANGLES, patch.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void SingleThreadApp::setupMatrices() {
    view_ = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);
    projection_ = glm::perspective(glm::radians(45.0f), 
                                  (float)windowWidth_ / (float)windowHeight_, 
                                  0.1f, 100.0f);
}

void SingleThreadApp::cleanup() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

// GLFW callbacks
void SingleThreadApp::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (s_instance_) {
        s_instance_->windowWidth_ = width;
        s_instance_->windowHeight_ = height;
        s_instance_->setupMatrices();
    }
}

void SingleThreadApp::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
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

void SingleThreadApp::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
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