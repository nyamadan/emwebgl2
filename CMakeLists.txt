cmake_minimum_required(VERSION 3.0.0)
project(emwebgl2 VERSION 0.0.0)
set(CMAKE_EXECUTABLE_SUFFIX .html)
set(CMAKE_C_FLAGS "\
    -Wall \
    -O3 \
    -g0 \
    -s NO_EXIT_RUNTIME=1 \
    -s FULL_ES2=1 \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s WASM=1 \
")
include_directories(Common)
add_executable(emwebgl2 main.c Common/esShader.c Common/esShapes.c Common/esTransform.c Common/esUtil.c)
