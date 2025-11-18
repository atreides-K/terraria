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
    utils/ShaderLoader.cpp
    src/LodManager.cpp
    src/BuildingManager.cpp
  )
  add_executable(app ${APP_SOURCES})
  

target_include_directories(app PRIVATE includes utils libs/earcut.hpp/include)
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")


set(DAWN_FETCH_DEPENDENCIES ON)
set(DAWN_BUILD_MONOLITHIC_LIBRARY STATIC)
add_subdirectory(libs/json)
set(EARCUT_BUILD_TESTS OFF CACHE BOOL "Disable building tests for earcut")
set(EARCUT_BUILD_BENCHMARKS OFF CACHE BOOL "Disable building benchmarks for earcut")

add_subdirectory("dawn" EXCLUDE_FROM_ALL)

if(EMSCRIPTEN)
  set_target_properties(app PROPERTIES SUFFIX ".html")
  target_compile_options(app PRIVATE -fexceptions)
  target_link_libraries(app PRIVATE emdawnwebgpu_cpp webgpu_glfw glm nlohmann_json::nlohmann_json )
  target_link_options(app PRIVATE
    "-sALLOW_MEMORY_GROWTH=1"

    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod0.hdr@/data/elevation_lod0.hdr"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod0.raw@/data/elevation_lod0.raw"

    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod1.hdr@/data/elevation_lod1.hdr"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod1.raw@/data/elevation_lod1.raw"

    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod2.hdr@/data/elevation_lod2.hdr"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod2.raw@/data/elevation_lod2.raw"

    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod3.hdr@/data/elevation_lod3.hdr"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod3.raw@/data/elevation_lod3.raw"

    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod4.hdr@/data/elevation_lod4.hdr"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/dem/lods/elevation_lod4.raw@/data/elevation_lod4.raw"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/osm/export.geojson@/data/buildings.geojson"
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/shaders/mesh.wgsl@/data/mesh.wgsl"
    "-sASYNCIFY=1"
    "-fexceptions"
    "-sUSE_GLFW=3"
    )
  
else()

  target_link_libraries(app PRIVATE webgpu_dawn webgpu_glfw glfw glm nlohmann_json::nlohmann_json )
endif()