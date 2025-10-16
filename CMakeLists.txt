cmake_minimum_required(VERSION 3.22) # CMake version check
project(app)                         # Create project "app"
include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.2
)
FetchContent_MakeAvailable(glm)

set(CMAKE_CXX_STANDARD 20)           # Enable C++20 standard


set(APP_SOURCES
    src/main.cpp
    src/Camera.cpp
    src/BufferUtils.cpp
)
add_executable(app ${APP_SOURCES})


target_include_directories(app PRIVATE includes)

set(DAWN_FETCH_DEPENDENCIES ON)
set(DAWN_BUILD_MONOLITHIC_LIBRARY STATIC)
add_subdirectory("dawn" EXCLUDE_FROM_ALL)

if(EMSCRIPTEN)
  set_target_properties(app PROPERTIES SUFFIX ".html")
  target_link_libraries(app PRIVATE emdawnwebgpu_cpp webgpu_glfw glm)
  target_link_options(app PRIVATE "-sASYNCIFY=1" "-sUSE_GLFW=3")
else()
  target_link_libraries(app PRIVATE webgpu_dawn webgpu_glfw glfw glm)
endif()