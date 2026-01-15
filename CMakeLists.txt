cmake_minimum_required(VERSION 3.16)
project(OpenGL_MultiThread_Test VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_SINGLE_THREAD "Build single-thread test" ON)
option(BUILD_MULTI_THREAD "Build multi-thread test" ON)
option(ENABLE_PERFORMANCE_MONITORING "Enable performance monitoring" ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

# Try to find GLFW3
pkg_check_modules(GLFW3 glfw3)
if(NOT GLFW3_FOUND)
    find_package(glfw3 REQUIRED)
    set(GLFW3_LIBRARIES glfw)
    set(GLFW3_INCLUDE_DIRS "")
endif()

# Find GLEW
find_package(GLEW REQUIRED)
if(NOT GLEW_FOUND)
    # Try alternative GLEW finding
    find_path(GLEW_INCLUDE_DIRS GL/glew.h)
    find_library(GLEW_LIBRARIES GLEW glew32)
    if(GLEW_INCLUDE_DIRS AND GLEW_LIBRARIES)
        set(GLEW_FOUND TRUE)
    endif()
endif()

# Find GLM (header-only library)
find_package(glm QUIET)
if(NOT glm_FOUND)
    find_path(GLM_INCLUDE_DIRS glm/glm.hpp
        PATHS /usr/include /usr/local/include
        PATH_SUFFIXES include)
    if(GLM_INCLUDE_DIRS)
        set(glm_FOUND TRUE)
        set(glm_INCLUDE_DIRS ${GLM_INCLUDE_DIRS})
    endif()
endif()

# Find threads
find_package(Threads REQUIRED)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared/include)

# Add external dependencies
add_subdirectory(external)

# Add shared library
add_subdirectory(shared)

# Add test executables
if(BUILD_SINGLE_THREAD)
    add_subdirectory(single_thread_test)
endif()

if(BUILD_MULTI_THREAD)
    # add_subdirectory(multi_thread_test)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "OpenGL Multi-Thread Test Configuration:")
message(STATUS "  Build Single Thread: ${BUILD_SINGLE_THREAD}")
message(STATUS "  Build Multi Thread: ${BUILD_MULTI_THREAD}")
message(STATUS "  Performance Monitoring: ${ENABLE_PERFORMANCE_MONITORING}")
message(STATUS "  OpenGL Found: ${OPENGL_FOUND}")
message(STATUS "  GLFW3 Found: ${GLFW3_FOUND}")
message(STATUS "  GLEW Found: ${GLEW_FOUND}")
message(STATUS "  GLM Found: ${glm_FOUND}")
if(OPENGL_INCLUDE_DIRS)
    message(STATUS "  OpenGL Include: ${OPENGL_INCLUDE_DIRS}")
endif()
if(GLFW3_INCLUDE_DIRS)
    message(STATUS "  GLFW3 Include: ${GLFW3_INCLUDE_DIRS}")
endif()
if(GLEW_INCLUDE_DIRS)
    message(STATUS "  GLEW Include: ${GLEW_INCLUDE_DIRS}")
endif()
if(glm_INCLUDE_DIRS)
    message(STATUS "  GLM Include: ${glm_INCLUDE_DIRS}")
endif()
message(STATUS "")
