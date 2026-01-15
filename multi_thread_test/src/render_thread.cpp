#include "render_thread.h"
#include <iostream>
#include <GLFW/glfw3.h>

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
    // For now, we'll implement a simplified version
    RenderTask task(RenderTask::UPLOAD_PATCH, patchId, nullptr, 0);
    submitTask(task);
    
    std::cout << "Submitted patch upload task for patch " << patchId << std::endl;
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
    switch (task.type) {
        case RenderTask::UPLOAD_PATCH:
            uploadPatchData(task.patchId, task.data, task.dataSize, nullptr, 0);
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
    // This is a simplified implementation
    // In a real implementation, this would upload vertex/index data to GPU
    
    std::cout << "Worker thread: Uploading patch " << patchId << " data (" 
              << vertexSize << " vertices, " << indexSize << " indices)" << std::endl;
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    std::cout << "Worker thread: Patch " << patchId << " upload completed!" << std::endl;
}