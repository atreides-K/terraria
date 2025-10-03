cmake_minimum_required(VERSION 3.16)
project(MyWebGPUProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the path to your pre-built Dawn directory.
# Example: cmake -B build -DDAWN_DIR=$HOME/lib/dawn/out/Debug
set(DAWN_DIR "" CACHE PATH "Path to the pre-built Dawn directory (e.g., dawn/out/Debug)")

if(NOT IS_DIRECTORY ${DAWN_DIR})
    message(FATAL_ERROR "DAWN_DIR is not a valid directory. Please set it to your pre-built Dawn location.")
endif()

# --- Define the list of include directories needed by Dawn ---
# We define this once to reuse it and keep things clean.
set(DAWN_INCLUDE_DIRS
    "${DAWN_DIR}/../../include"      # Path to the main dawn/include
    "${DAWN_DIR}/gen/include"        # Path to the generated headers in your build dir
)


# --- webgpu_dawn library ---
add_library(webgpu_dawn STATIC IMPORTED)
set_target_properties(webgpu_dawn PROPERTIES
    IMPORTED_LOCATION "${DAWN_DIR}/src/dawn/native/libwebgpu_dawn.a"
    # THE FIX: Provide the include directories as a single, semicolon-separated string.
    INTERFACE_INCLUDE_DIRECTORIES "${DAWN_INCLUDE_DIRS}"
)


# --- dawn_proc library ---
add_library(dawn_proc STATIC IMPORTED)
set_target_properties(dawn_proc PROPERTIES
    IMPORTED_LOCATION "${DAWN_DIR}/src/dawn/libdawn_proc.a"
    # THE FIX: This library ALSO needs the include directories.
    INTERFACE_INCLUDE_DIRECTORIES "${DAWN_INCLUDE_DIRS}"
)


# --- Define and link your application ---
add_executable(App main.cpp)

# This now works correctly because both imported targets properly
# advertise their include directory requirements.
target_link_libraries(App PRIVATE
    webgpu_dawn
    dawn_proc
)