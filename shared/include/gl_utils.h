#pragma once

// GLEW must be included before any other OpenGL headers
#include <GL/glew.h>

#include <string>
#include <vector>
#include <glm/glm.hpp>


struct Shader {
    GLuint program = 0;
    
    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;
    void dispose();
    
    // Uniform setters
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& value) const;
    
    bool isValid() const { return program != 0; }
    
private:
    GLuint compileShader(const std::string& source, GLenum type);
    std::string loadShaderSource(const std::string& filePath);
};

class GLUtils {
public:
    // Error checking
    static void checkOpenGLError(const std::string& operation = "");
    static void enableDebugOutput();
    
    // Utility functions
    static std::string getOpenGLVersion();
    static std::string getRendererName();
    static std::string getVendorName();
    
    // Texture loading utilities
    static GLuint createTexture2D(int width, int height, GLenum format = GL_RGB);
    static GLuint loadTexture(const std::string& filePath);
    
    // Framebuffer utilities
    static GLuint createFramebuffer(GLuint colorAttachment, GLuint depthAttachment = 0);
    static void deleteFramebuffer(GLuint fbo);
    
    // VAO/VBO utilities
    static GLuint createVAO();
    static void deleteVAO(GLuint vao);
    static GLuint createVBO();
    static void deleteVBO(GLuint vbo);
    static GLuint createEBO();
    static void deleteEBO(GLuint ebo);
    
    // Timer queries for GPU timing
    static GLuint createTimerQuery();
    static void beginTimerQuery(GLuint query);
    static void endTimerQuery(GLuint query);
    static double getTimerResult(GLuint query);
    static void deleteTimerQuery(GLuint query);
    
private:
    static void debugCallback(GLenum source, GLenum type, GLuint id,
                                       GLenum severity, GLsizei length, 
                                       const GLchar* message, const void* userParam);
};
