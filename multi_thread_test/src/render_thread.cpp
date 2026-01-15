#include "render_thread.h"
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../../shared/include/terrain_generator.h"

RenderThread::RenderThread()
    : workerContext_(nullptr), contextInitialized_(false),
      isRunning_(false), shouldStop_(false), processedTasks_(0) {
}

RenderThread::~RenderThread() {
    stop();
}

bool RenderThread::initialize(GLFWwindow* sharedContext) {
    if (isRunning_) {
        std::cerr << "Render thread is already running!" << std::endl;
        return false;
    }
    
    // Create worker context (shared with main context)
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    workerContext_ = glfwCreateWindow(1, 1, "Worker Context", NULL, sharedContext);
    
    if (!workerContext_) {
        std::cerr << "Failed to create worker OpenGL context!" << std::endl;
        return false;
    }
    
    std::cout << "Render thread initialized successfully!" << std::endl;
    return true;
}

void RenderThread::start() {
    if (isRunning_) {
        std::cerr << "Render thread is already running!" << std::endl;
        return;
    }
    
    isRunning_ = true;
    shouldStop_ = false;
    processedTasks_ = 0;
    
    workerThread_ = std::thread(&RenderThread::threadFunction, this);
    
    std::cout << "Render thread started!" << std::endl;
}

void RenderThread::stop() {
    if (!isRunning_) {
        return;
    }
    
    shouldStop_ = true;
    
    // Notify the thread to wake up
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queueCondition_.notify_one();
    }
    
    // Wait for thread to finish
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
    
    // Clean up worker context
    if (workerContext_) {
        glfwDestroyWindow(workerContext_);
        workerContext_ = nullptr;
    }
    
    isRunning_ = false;
    std::cout << "Render thread stopped. Processed " << processedTasks_.load() << " tasks." << std::endl;
}

void RenderThread::submitTask(const RenderTask& task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        taskQueue_.push(task);
    }
    queueCondition_.notify_one();
}

void RenderThread::submitPatchUpload(int patchId, void* vertexData, size_t vertexSize,
                                    void* indexData, size_t indexSize) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        // Store vertex data and size, and pass index data by storing size in upper bits
        // We'll use a simple encoding: store indexSize in high 32 bits, vertexSize in low 32 bits
        uint64_t encodedSize = (static_cast<uint64_t>(indexSize) << 32) | static_cast<uint64_t>(vertexSize);
        taskQueue_.push(RenderTask(RenderTask::UPLOAD_PATCH, patchId, vertexData, encodedSize));
    }
    queueCondition_.notify_one();
}

void RenderThread::waitForCompletion() {
    std::unique_lock<std::mutex> lock(completionMutex_);
    completionCondition_.wait(lock, [this] { return taskQueue_.empty(); });
}

bool RenderThread::hasPendingTasks() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return !taskQueue_.empty();
}

size_t RenderThread::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex_);
    return taskQueue_.size();
}

void RenderThread::threadFunction() {
    // Make this thread's OpenGL context current
    glfwMakeContextCurrent(workerContext_);
    
    // Initialize OpenGL state for this thread
    if (!contextInitialized_) {
        // Initialize GLEW for this thread
        // glewExperimental = GL_TRUE;
        // if (glewInit() != GLEW_OK) {
        //     std::cerr << "Failed to initialize GLEW in render thread!" << std::endl;
        //     return;
        // }
        
        contextInitialized_ = true;
        std::cout << "Render thread OpenGL context initialized!" << std::endl;
    }
    
    // Main worker loop
    while (!shouldStop_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        
        // Wait for tasks or stop signal
        queueCondition_.wait(lock, [this] { return !taskQueue_.empty() || shouldStop_; });
        
        // Process all available tasks
        while (!taskQueue_.empty() && !shouldStop_) {
            RenderTask task = taskQueue_.front();
            taskQueue_.pop();
            lock.unlock();
            
            // Process the task
            processTask(task);
            processedTasks_++;
            
            lock.lock();
        }
        
        // Notify that we've completed all tasks
        if (taskQueue_.empty()) {
            lock.unlock();
            {
                std::lock_guard<std::mutex> completionLock(completionMutex_);
                completionCondition_.notify_all();
            }
        }
    }
    
    // Clean up
    glfwMakeContextCurrent(nullptr);
}

void RenderThread::processTask(const RenderTask& task) {
    // Decode the encoded sizes
    size_t vertexSize = static_cast<size_t>(task.dataSize & 0xFFFFFFFF);
    size_t indexSize = static_cast<size_t>(task.dataSize >> 32);
    
    switch (task.type) {
        case RenderTask::UPLOAD_PATCH:
            uploadPatchData(task.patchId, task.data, vertexSize, 
                              reinterpret_cast<void*>(indexSize), indexSize);
            break;
            
        case RenderTask::UPDATE_BUFFER:
            // Handle buffer updates
            break;
            
        case RenderTask::RENDER_PATCH:
            // Handle patch rendering
            break;
            
        case RenderTask::CLEANUP:
            // Handle cleanup tasks
            break;
            
        default:
            std::cerr << "Unknown task type: " << task.type << std::endl;
            break;
    }
}

void RenderThread::uploadPatchData(int patchId, void* vertexData, size_t vertexSize,
                                void* indexData, size_t indexSize) {
    // Actually upload vertex/index data to GPU
    
    // Convert byte sizes to vertex/index counts
    size_t vertexCount = vertexSize / sizeof(TerrainVertex);
    size_t indexCount = indexSize / sizeof(unsigned int);
    
    std::cout << "Worker thread: Uploading patch " << patchId << " data (" 
              << vertexCount << " vertices, " << indexCount << " indices)" << std::endl;
    
    // Create GPU resources for this patch
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Upload vertex data (vertexSize is already in bytes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexSize, vertexData, GL_STATIC_DRAW);
    
    // Upload index data (indexSize is already in bytes)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indexData, GL_STATIC_DRAW);
    
    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 
                        (void*)offsetof(TerrainVertex, position));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                        (void*)offsetof(TerrainVertex, normal));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                        (void*)offsetof(TerrainVertex, texCoord));
    glEnableVertexAttribArray(2);
    
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
                        (void*)offsetof(TerrainVertex, color));
    glEnableVertexAttribArray(3);
    
    // Store the patch data (in a real implementation, this would be more sophisticated)
    // For now, we'll just complete the upload
    glBindVertexArray(0);
    
    std::cout << "Worker thread: Patch " << patchId << " upload completed!" << std::endl;
}