#include "gl_utils.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
// #include <GL/glew.h>
// #include <GL/gl.h>
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    dispose();
}

void Shader::load(const std::string& vertexPath, const std::string& fragmentPath) {
    // Clean up existing shader
    if (program != 0) {
        glDeleteProgram(program);
    }
    
    // Load shader sources
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);
    
    // Compile shaders
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    
    // Create and link program
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n" << infoLog << std::endl;
        program = 0;
    }
    
    // Clean up individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() const {
    if (program != 0) {
        glUseProgram(program);
    }
}

void Shader::dispose() {
    if (program != 0) {
        glDeleteProgram(program);
        program = 0;
    }
}

void Shader::setInt(const std::string& name, int value) const {
    if (program != 0) {
        glUniform1i(glGetUniformLocation(program, name.c_str()), value);
    }
}

void Shader::setFloat(const std::string& name, float value) const {
    if (program != 0) {
        glUniform1f(glGetUniformLocation(program, name.c_str()), value);
    }
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    if (program != 0) {
        glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    if (program != 0) {
        glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    if (program != 0) {
        glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    if (program != 0) {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }
}

GLuint Shader::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);
    
    // Check compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

std::string Shader::loadShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// GLUtils implementation
void GLUtils::checkOpenGLError(const std::string& operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error";
        if (!operation.empty()) {
            std::cerr << " (" << operation << ")";
        }
        std::cerr << ": 0x" << std::hex << error << std::dec << std::endl;
    }
}

void GLUtils::enableDebugOutput() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

void GLUtils::debugCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length, 
                                     const GLchar* message, const void* userParam) {
    // Ignore some non-critical messages
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    
    std::cerr << "OpenGL Debug Message (" << id << "): " << message << std::endl;
    
    switch (source) {
        case GL_DEBUG_SOURCE_API: std::cerr << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cerr << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: std::cerr << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: std::cerr << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER: std::cerr << "Source: Other"; break;
    }
    
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: std::cerr << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behavior"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cerr << "Type: Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: std::cerr << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: std::cerr << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER: std::cerr << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: std::cerr << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP: std::cerr << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER: std::cerr << "Type: Other"; break;
    }
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: std::cerr << "Severity: High"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: std::cerr << "Severity: Medium"; break;
        case GL_DEBUG_SEVERITY_LOW: std::cerr << "Severity: Low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: Notification"; break;
    }
    
    std::cerr << std::endl;
}

std::string GLUtils::getOpenGLVersion() {
    const GLubyte* version = glGetString(GL_VERSION);
    return std::string(reinterpret_cast<const char*>(version));
}

std::string GLUtils::getRendererName() {
    const GLubyte* renderer = glGetString(GL_RENDERER);
    return std::string(reinterpret_cast<const char*>(renderer));
}

std::string GLUtils::getVendorName() {
    const GLubyte* vendor = glGetString(GL_VENDOR);
    return std::string(reinterpret_cast<const char*>(vendor));
}

GLuint GLUtils::createTexture2D(int width, int height, GLenum format) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLuint GLUtils::createVAO() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    return vao;
}

void GLUtils::deleteVAO(GLuint vao) {
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
    }
}

GLuint GLUtils::createVBO() {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    return vbo;
}

void GLUtils::deleteVBO(GLuint vbo) {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }
}

GLuint GLUtils::createEBO() {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    return ebo;
}

void GLUtils::deleteEBO(GLuint ebo) {
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
    }
}

GLuint GLUtils::createTimerQuery() {
    GLuint query;
    glGenQueries(1, &query);
    return query;
}

void GLUtils::beginTimerQuery(GLuint query) {
    glBeginQuery(GL_TIME_ELAPSED, query);
}

void GLUtils::endTimerQuery(GLuint query) {
    glEndQuery(GL_TIME_ELAPSED);
}

double GLUtils::getTimerResult(GLuint query) {
    GLuint64 result;
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &result);
    return static_cast<double>(result) / 1e9; // Convert nanoseconds to seconds
}

void GLUtils::deleteTimerQuery(GLuint query) {
    if (query != 0) {
        glDeleteQueries(1, &query);
    }
}
