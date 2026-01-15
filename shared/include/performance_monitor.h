#pragma once

#include <vector>
#include <chrono>
#include <memory>
#include <mutex>

struct PerformanceMetrics {
    // Frame metrics
    double fps = 0.0;
    double frameTime = 0.0;
    double minFrameTime = 1000.0;
    double maxFrameTime = 0.0;
    
    // Rendering metrics
    int drawCalls = 0;
    int trianglesDrawn = 0;
    int verticesDrawn = 0;
    
    // Memory metrics
    size_t memoryUsage = 0;
    size_t vboMemory = 0;
    size_t textureMemory = 0;
    
    // CPU metrics (approximate)
    double cpuUsage = 0.0;
    
    // Timing
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    std::chrono::high_resolution_clock::time_point startTime;
    
    PerformanceMetrics() {
        lastFrameTime = std::chrono::high_resolution_clock::now();
        startTime = lastFrameTime;
    }
};

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor() = default;
    
    // Frame management
    void beginFrame();
    void endFrame();
    void incrementDrawCalls(int count = 1) { metrics_.drawCalls += count; }
    void addTriangles(int count) { metrics_.trianglesDrawn += count; }
    void addVertices(int count) { metrics_.verticesDrawn += count; }
    void addMemoryUsage(size_t bytes) { metrics_.memoryUsage += bytes; }
    void addVBOMemory(size_t bytes) { metrics_.vboMemory += bytes; }
    void addTextureMemory(size_t bytes) { metrics_.textureMemory += bytes; }
    
    // Accessors
    const PerformanceMetrics& getMetrics() const { return metrics_; }
    double getFPS() const { return metrics_.fps; }
    double getFrameTime() const { return metrics_.frameTime; }
    int getDrawCalls() const { return metrics_.drawCalls; }
    
    // Statistics
    void reset();
    void printReport() const;
    
    // Utility functions
    static std::string formatBytes(size_t bytes);
    static std::string formatTime(double milliseconds);
    
private:
    PerformanceMetrics metrics_;
    mutable std::mutex metricsMutex_;
    
    // Frame time calculation
    std::vector<double> frameTimeHistory_;
    static constexpr size_t FRAME_HISTORY_SIZE = 60; // 60-frame rolling average
    
    void updateFPS();
    void updateFrameTime();
};