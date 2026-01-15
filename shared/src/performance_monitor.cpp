#include "performance_monitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>

PerformanceMonitor::PerformanceMonitor() : frameTimeHistory_(FRAME_HISTORY_SIZE) {
}

void PerformanceMonitor::beginFrame() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    metrics_.lastFrameTime = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::endFrame() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - metrics_.lastFrameTime);
    double frameTimeMs = duration.count() / 1000.0;
    
    // Update frame time history
    frameTimeHistory_.push_back(frameTimeMs);
    if (frameTimeHistory_.size() > FRAME_HISTORY_SIZE) {
        frameTimeHistory_.erase(frameTimeHistory_.begin());
    }
    
    // Update min/max frame times
    metrics_.minFrameTime = std::min(metrics_.minFrameTime, frameTimeMs);
    metrics_.maxFrameTime = std::max(metrics_.maxFrameTime, frameTimeMs);
    
    // Update FPS and frame time
    updateFPS();
    updateFrameTime();
}

void PerformanceMonitor::updateFPS() {
    if (frameTimeHistory_.size() < 2) return;
    
    // Calculate average frame time over the history
    double totalTime = 0.0;
    for (double time : frameTimeHistory_) {
        totalTime += time;
    }
    double avgFrameTime = totalTime / frameTimeHistory_.size();
    
    // Convert to FPS
    if (avgFrameTime > 0.0) {
        metrics_.fps = 1000.0 / avgFrameTime;
    }
}

void PerformanceMonitor::updateFrameTime() {
    if (frameTimeHistory_.empty()) return;
    
    double totalTime = 0.0;
    for (double time : frameTimeHistory_) {
        totalTime += time;
    }
    metrics_.frameTime = totalTime / frameTimeHistory_.size();
}

void PerformanceMonitor::reset() {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    metrics_ = PerformanceMetrics();
    frameTimeHistory_.clear();
}

void PerformanceMonitor::printReport() const {
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    auto now = std::chrono::high_resolution_clock::now();
    auto totalRuntime = std::chrono::duration_cast<std::chrono::seconds>(now - metrics_.startTime);
    
    std::cout << "\n=== Performance Report ===" << std::endl;
    std::cout << "Total Runtime: " << totalRuntime.count() << " seconds" << std::endl;
    std::cout << "Average FPS: " << std::fixed << std::setprecision(2) << metrics_.fps << std::endl;
    std::cout << "Average Frame Time: " << formatTime(metrics_.frameTime) << std::endl;
    std::cout << "Min Frame Time: " << formatTime(metrics_.minFrameTime) << std::endl;
    std::cout << "Max Frame Time: " << formatTime(metrics_.maxFrameTime) << std::endl;
    
    std::cout << "\nRendering Statistics:" << std::endl;
    std::cout << "Draw Calls: " << metrics_.drawCalls << std::endl;
    std::cout << "Triangles Drawn: " << metrics_.trianglesDrawn << std::endl;
    std::cout << "Vertices Drawn: " << metrics_.verticesDrawn << std::endl;
    
    std::cout << "\nMemory Usage:" << std::endl;
    std::cout << "Total Memory: " << formatBytes(metrics_.memoryUsage) << std::endl;
    std::cout << "VBO Memory: " << formatBytes(metrics_.vboMemory) << std::endl;
    std::cout << "Texture Memory: " << formatBytes(metrics_.textureMemory) << std::endl;
    
    if (totalRuntime.count() > 0) {
        double avgDrawCallsPerSecond = static_cast<double>(metrics_.drawCalls) / totalRuntime.count();
        double avgTrianglesPerSecond = static_cast<double>(metrics_.trianglesDrawn) / totalRuntime.count();
        
        std::cout << "\nThroughput:" << std::endl;
        std::cout << "Avg Draw Calls/sec: " << std::fixed << std::setprecision(1) << avgDrawCallsPerSecond << std::endl;
        std::cout << "Avg Triangles/sec: " << std::fixed << std::setprecision(0) << avgTrianglesPerSecond << std::endl;
    }
    
    std::cout << "========================\n" << std::endl;
}

std::string PerformanceMonitor::formatBytes(size_t bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int suffixIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && suffixIndex < 4) {
        size /= 1024.0;
        suffixIndex++;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << size << " " << suffixes[suffixIndex];
    return ss.str();
}

std::string PerformanceMonitor::formatTime(double milliseconds) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << milliseconds << " ms";
    return ss.str();
}