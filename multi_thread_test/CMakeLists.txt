# Multi-threaded test executable

add_executable(multi_thread_test
    src/main.cpp
    src/multi_thread_app.cpp
    src/render_thread.cpp
)

target_include_directories(multi_thread_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(multi_thread_test
    shared
)

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../shared/shaders DESTINATION ${CMAKE_BINARY_DIR})