# Shared library for common utilities

add_library(shared STATIC
    src/terrain_generator.cpp
    src/performance_monitor.cpp
    src/gl_utils.cpp
)

target_include_directories(shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIRS}
    ${GLFW3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${glm_INCLUDE_DIRS}
)

target_link_libraries(shared
    opengl
    glfw
    glew
    glm
    ${CMAKE_THREAD_LIBS_INIT}
)

if(WIN32)
    target_link_libraries(shared opengl32)
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/shared)