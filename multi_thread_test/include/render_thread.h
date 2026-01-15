#pragma once

// Include GLEW first before GLFW to avoid header conflicts
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <memory>

struct RenderTask {
    enum Type {
        UPLOAD_PATCH,
        UPDATE_BUFFER,
        RENDER_PATCH,
        CLEANUP
    };
    
    Type type;
    int patchId;
    void* data;
    size_t dataSize;
    
    RenderTask(Type t = RENDER_PATCH, int id = -1, void* d = nullptr, size_t s = 0)
        : type(t), patchId(id), data(d), dataSize(s) {}
};

class RenderThread {
public:
    RenderThread();
    ~RenderThread();
    
    // Thread management
    bool initialize(GLFWwindow* sharedContext);
    void start();
    void stop();
    bool isRunning() const { return isRunning_.load(); }
    
    // Task submission
    void submitTask(const RenderTask& task);
    void submitPatchUpload(int patchId, void* vertexData, size_t vertexSize, void* indexData, size_t indexSize);
    
    // Synchronization
    void waitForCompletion();
    bool hasPendingTasks() const;
    
    // Statistics
    size_t getProcessedTasks() const { return processedTasks_.load(); }
    size_t getQueueSize() const;
    
private:
    // Thread function
    void threadFunction();
    
    // Task processing
    void processTask(const RenderTask& task);
    void uploadPatchData(int patchId, void* vertexData, size_t vertexSize, void* indexData, size_t indexSize);
    
    // OpenGL context
    GLFWwindow* workerContext_;
    bool contextInitialized_;
    
    // Threading
    std::thread workerThread_;
    std::atomic<bool> isRunning_;
    std::atomic<bool> shouldStop_;
    
    // Task queue
    std::queue<RenderTask> taskQueue_;
    mutable std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    
    // Statistics
    std::atomic<size_t> processedTasks_;
    
    // Synchronization
    std::condition_variable completionCondition_;
    mutable std::mutex completionMutex_;
};
