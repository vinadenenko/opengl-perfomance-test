# External dependencies configuration

# GLFW3 setup
if(GLFW3_FOUND AND GLFW3_LIBRARIES)
    # System-provided GLFW3
    add_library(glfw INTERFACE)
    target_include_directories(glfw INTERFACE ${GLFW3_INCLUDE_DIRS})
    target_link_libraries(glfw INTERFACE ${GLFW3_LIBRARIES})
else()
    # Fallback: assume GLFW is available
    message(WARNING "GLFW3 not found via pkg-config, assuming system installation")
    add_library(glfw INTERFACE)
    target_link_libraries(glfw INTERFACE glfw)
endif()

# GLEW setup
if(GLEW_FOUND)
    add_library(glew INTERFACE)
    target_include_directories(glew INTERFACE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(glew INTERFACE ${GLEW_LIBRARIES})
else()
    message(WARNING "GLEW not found, using fallback")
    add_library(glew INTERFACE)
    target_link_libraries(glew INTERFACE GLEW)
endif()

# GLM setup (header-only)
if(glm_FOUND)
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE ${glm_INCLUDE_DIRS})
else()
    message(WARNING "GLM not found, using fallback")
    add_library(glm INTERFACE)
    target_include_directories(glm INTERFACE /usr/include /usr/local/include)
endif()

# OpenGL setup
add_library(opengl INTERFACE)
target_include_directories(opengl INTERFACE ${OPENGL_INCLUDE_DIRS})
target_link_libraries(opengl INTERFACE ${OPENGL_LIBRARIES})