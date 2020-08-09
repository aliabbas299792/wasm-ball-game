cmake_minimum_required(VERSION 3.16)
project(OpenGL_WASM_Project)

set(CMAKE_CXX_STANDARD 14)

include_directories(public/src)
include_directories(public/include)
include_directories(public/assets)
include_directories(public/emsdk/upstream/emscripten/system/include)

#this recursively gets all .cpp and .h files
file(GLOB_RECURSE sourceFiles "public/src/*.cpp" "public/src/*.h")
#gets all static libraries
file(GLOB_RECURSE staticLibraries "public/libs/*.a")

#below adds the source files found via the file() command above
add_executable(OpenGL_WASM_Project ${sourceFiles})
#below links the static libraries found via the file() command above
target_link_libraries(OpenGL_WASM_Project ${staticLibraries})

option(OptimisationLevel "OptimisationLevel" 0)

set_target_properties(
    OpenGL_WASM_Project
    PROPERTIES SUFFIX ".js"
    LINK_FLAGS "-s WASM=1 \
    -s USE_GLFW=3 \
    -s FULL_ES3=1 \
    -s USE_WEBGL2=1 \
    -s MIN_WEBGL_VERSION=2 \
    -O${OptimisationLevel} \
    -o index.js \
    -s ALLOW_MEMORY_GROWTH=1 \
    --no-heap-copy \
    --preload-file '${PROJECT_SOURCE_DIR}/public/assets/images@/images' \
    --preload-file '${PROJECT_SOURCE_DIR}/public/assets/models@/models' \
    --js-library '${PROJECT_SOURCE_DIR}/public/library.js' \
    -s LLD_REPORT_UNDEFINED"
)