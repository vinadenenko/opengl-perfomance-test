#include "terrain_generator.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>

TerrainGenerator::TerrainGenerator(int gridSize, float patchSize, float heightScale)
    : gridSize_(gridSize), patchSize_(patchSize), heightScale_(heightScale), noiseInitialized_(false) {
    initializeNoise();
}

TerrainGenerator::~TerrainGenerator() {
    // Clean up OpenGL resources if they were created
    for (auto& patch : patches_) {
        if (patch.VAO != 0) {
            glDeleteVertexArrays(1, &patch.VAO);
            glDeleteBuffers(1, &patch.VBO);
            glDeleteBuffers(1, &patch.EBO);
        }
    }
}

void TerrainGenerator::initializeNoise() {
    if (noiseInitialized_) return;
    
    // Initialize permutation table for Perlin noise
    permutation_.resize(256);
    std::iota(permutation_.begin(), permutation_.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(permutation_.begin(), permutation_.end(), g);
    
    // Duplicate for overflow
    permutation_.insert(permutation_.end(), permutation_.begin(), permutation_.end());
    noiseInitialized_ = true;
}

float TerrainGenerator::getHeight(float x, float z) const {
    return perlinNoise(x * 0.1f, z * 0.1f, 4, 0.5f) * heightScale_;
}

float TerrainGenerator::perlinNoise(float x, float z, int octaves, float persistence) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    
    for (int i = 0; i < octaves; i++) {
        total += grad(permutation_[static_cast<int>(x * frequency) & 255] + 
                     permutation_[static_cast<int>(z * frequency) & 255],
                     x * frequency, z * frequency) * amplitude;
        
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    
    return total / maxValue;
}

float TerrainGenerator::grad(int hash, float x, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : z;
    float v = h < 4 ? z : h == 12 || h == 14 ? x : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

glm::vec3 TerrainGenerator::calculateNormal(float x, float z) const {
    float delta = 0.1f;
    float hL = getHeight(x - delta, z);
    float hR = getHeight(x + delta, z);
    float hD = getHeight(x, z - delta);
    float hU = getHeight(x, z + delta);
    
    glm::vec3 normal(hL - hR, 2.0f * delta, hD - hU);
    return glm::normalize(normal);
}

void TerrainGenerator::generateTerrain() {
    patches_.clear();
    
    // Generate terrain patches
    const int patchesPerRow = std::sqrt(64); // 8x8 patches
    const int patchVertexSize = gridSize_ / patchesPerRow;
    
    for (int row = 0; row < patchesPerRow; row++) {
        for (int col = 0; col < patchesPerRow; col++) {
            TerrainPatch patch;
            createPatch(col * patchVertexSize, row * patchVertexSize, patchVertexSize, patch);
            patches_.push_back(patch);
        }
    }
    
    std::cout << "Generated " << patches_.size() << " terrain patches" << std::endl;
    std::cout << "Total vertices: " << getTotalVertices() << std::endl;
    std::cout << "Total triangles: " << getTotalTriangles() << std::endl;
}

void TerrainGenerator::createPatch(int startX, int startZ, int patchSize, TerrainPatch& patch) {
    patch.vertices.clear();
    patch.indices.clear();
    
    // Generate vertices
    for (int z = startZ; z <= startZ + patchSize; z++) {
        for (int x = startX; x <= startX + patchSize; x++) {
            TerrainVertex vertex;
            float worldX = x * patchSize_;
            float worldZ = z * patchSize_;
            
            vertex.position = glm::vec3(worldX, getHeight(worldX, worldZ), worldZ);
            vertex.normal = calculateNormal(worldX, worldZ);
            vertex.texCoord = glm::vec2(static_cast<float>(x) / gridSize_, 
                                       static_cast<float>(z) / gridSize_);
            
            // Color based on height
            float heightFactor = (vertex.position.y + heightScale_) / (2.0f * heightScale_);
            heightFactor = glm::clamp(heightFactor, 0.0f, 1.0f);
            vertex.color = glm::mix(glm::vec3(0.2f, 0.5f, 0.1f),    // Dark green (valleys)
                                   glm::vec3(0.9f, 0.9f, 0.7f),     // Light gray (peaks)
                                   heightFactor);
            
            patch.vertices.push_back(vertex);
        }
    }
    
    // Generate indices (triangles)
    const int verticesPerRow = patchSize + 1;
    for (int z = 0; z < patchSize; z++) {
        for (int x = 0; x < patchSize; x++) {
            int topLeft = z * verticesPerRow + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * verticesPerRow + x;
            int bottomRight = bottomLeft + 1;
            
            // Two triangles per quad
            patch.indices.insert(patch.indices.end(), {
                topLeft, bottomLeft, topRight,
                topRight, bottomLeft, bottomRight
            });
        }
    }
    
    // Calculate patch center and bounding sphere
    patch.center = glm::vec3(0.0f);
    for (const auto& vertex : patch.vertices) {
        patch.center += vertex.position;
    }
    patch.center /= patch.vertices.size();
    
    patch.boundingRadius = 0.0f;
    for (const auto& vertex : patch.vertices) {
        float distance = glm::length(vertex.position - patch.center);
        patch.boundingRadius = std::max(patch.boundingRadius, distance);
    }
    
    patch.lodLevel = 0;
}

size_t TerrainGenerator::getTotalVertices() const {
    size_t total = 0;
    for (const auto& patch : patches_) {
        total += patch.vertices.size();
    }
    return total;
}

size_t TerrainGenerator::getTotalTriangles() const {
    size_t total = 0;
    for (const auto& patch : patches_) {
        total += patch.indices.size() / 3;
    }
    return total;
}