# Single-threaded test executable

add_executable(single_thread_test
    src/main.cpp
    src/single_thread_app.cpp
)

target_include_directories(single_thread_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(single_thread_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLFW3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

target_link_libraries(single_thread_test
    shared
    ${GLFW3_LDFLAGS}
    ${OPENGL_LIBRARIES}
)

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../shared/shaders DESTINATION ${CMAKE_BINARY_DIR})