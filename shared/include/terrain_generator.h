#pragma once

// GLEW must be included before any other OpenGL headers
#include <GL/glew.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TerrainVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 color;
};

struct TerrainPatch {
    std::vector<TerrainVertex> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 center;
    float boundingRadius;
    int lodLevel;
    
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    bool isUploaded = false;
};

class TerrainGenerator {
public:
    TerrainGenerator(int gridSize = 256, float patchSize = 1.0f, float heightScale = 20.0f);
    ~TerrainGenerator();

    // Generate terrain data
    void generateTerrain();
    void generatePatches(int patchCount = 64);
    
    // Accessors
    const std::vector<TerrainPatch>& getPatches() const { return patches_; }
    int getGridSize() const { return gridSize_; }
    float getHeightScale() const { return heightScale_; }
    
    // Statistics
    size_t getTotalVertices() const;
    size_t getTotalTriangles() const;
    
private:
    // Height generation
    float getHeight(float x, float z) const;
    float perlinNoise(float x, float z, int octaves = 4, float persistence = 0.5f) const;
    float fadeFunction(float t) const;
    float lerp(float a, float b, float t) const;
    float grad(int hash, float x, float z) const;
    
    // Normal calculation
    glm::vec3 calculateNormal(float x, float z) const;
    
    // Patch creation
    void createPatch(int startX, int startZ, int patchSize, TerrainPatch& patch);
    
    // Member variables
    int gridSize_;
    float patchSize_;
    float heightScale_;
    std::vector<TerrainPatch> patches_;
    
    // Perlin noise
    mutable std::vector<int> permutation_;
    bool noiseInitialized_;
    void initializeNoise();
};