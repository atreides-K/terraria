cmake_minimum_required(VERSION 3.22)
project(app)                       
include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.2
)
FetchContent_MakeAvailable(glm)

set(CMAKE_CXX_STANDARD 20)          
set(APP_SOURCES
    src/main.cpp
    src/Camera.cpp
    src/BufferUtils.cpp
    src/Mesh.cpp
    src/Pipeline.cpp
    utils/demLoader.cpp
)
add_executable(app ${APP_SOURCES})


target_include_directories(app PRIVATE includes utils)
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

set(DAWN_FETCH_DEPENDENCIES ON)
set(DAWN_BUILD_MONOLITHIC_LIBRARY STATIC)
add_subdirectory("dawn" EXCLUDE_FROM_ALL)

if(EMSCRIPTEN)
  set_target_properties(app PROPERTIES SUFFIX ".html")
  target_compile_options(app PRIVATE -fexceptions)
  target_link_libraries(app PRIVATE emdawnwebgpu_cpp webgpu_glfw glm)
  target_link_options(app PRIVATE
    "-sALLOW_MEMORY_GROWTH=1"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/output.hdr@/data/output.hdr"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/output.raw@/data/output.raw"
    "-sASYNCIFY=1"
    "-fexceptions"
    "-sUSE_GLFW=3"
    )
  
else()
  target_link_libraries(app PRIVATE webgpu_dawn webgpu_glfw glfw glm)
endif()